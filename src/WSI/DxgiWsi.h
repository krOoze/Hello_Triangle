// win32 platform dependent WSI handling and event loop

#ifndef COMMON_DXGI_WSI_H
#define COMMON_DXGI_WSI_H

#include <cassert>
#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <dxgi1_6.h>
#ifndef NDEBUG
	#include <dxgidebug.h>
	#include <d3d12sdklayers.h>
#endif
#include <d3d12.h>
#include <vulkan/vulkan.h>

#include "CompilerMessages.h"
#include "EnumerateScheme.h"
#include "ErrorHandling.h"


struct PlatformWindow{ HINSTANCE hInstance; HWND hWnd; };

DWORD windowedStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
DWORD windowedExStyle = WS_EX_OVERLAPPEDWINDOW;

struct PlatformSurfaceImpl{
	PlatformWindow window;
};

struct PlatformSwapchainImpl{
	PlatformSurfaceImpl surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
#ifndef NDEBUG
	ComPtr<IDXGIDebug1> dxgiDebug;
	ComPtr<IDXGIInfoQueue> dxgiDebugQueue;
	ComPtr<ID3D12Debug1> dx12Debug;
	ComPtr<ID3D12InfoQueue> dx12DebugQueue;
#endif
	ComPtr<IDXGIFactory7> dxgiFac;
	ComPtr<IDXGIAdapter4> dxgiAdapter;
	ComPtr<ID3D12Device5> dxDevice;
	ComPtr<ID3D12CommandQueue> dxCommandQueue;
	ComPtr<IDXGISwapChain4> dxgiSwapchain;

	uint32_t imageCount;
	std::vector<VkImage> swapchainImages;
	std::vector<VkDeviceMemory> importedMemories;
	std::vector<HANDLE> sharedHandles;
};

using PlatformSurface = uint64_t;
using PlatformSwapchain = uint64_t;

std::unordered_map<PlatformSurface, PlatformSurfaceImpl> platformSurfaces;
PlatformSurface nextFreeSurfaceHandle = 1;
std::unordered_map<PlatformSwapchain, PlatformSwapchainImpl> platformSwapchains;
PlatformSurface nextFreeSwapchainHandle = 1;

TODO( "We shouldn't depend on this specific format" )
VkFormat extImgFormat = VK_FORMAT_B8G8R8A8_UNORM;

VkExtent2D currentWndSize;

std::vector<const char*> getPlatformSurfaceExtensionNames(){
	return {
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME
	};
};

const std::vector<const char*> requiredDeviceExtensions = {
	VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
	VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
	VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
	VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
};
std::vector<const char*> getPlatformSwapchainExtensionNames(){
	return ::requiredDeviceExtensions;
}

int messageLoop( PlatformWindow window );

bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow window );

PlatformWindow initWindow( const std::string& name, uint32_t canvasWidth, uint32_t canvasHeight );
void killWindow( PlatformWindow window );

PlatformSurface initSurface( VkInstance instance, PlatformWindow window );
void killSurface( VkInstance instance, PlatformSurface surface );

bool isPresentationSupported( const VkPhysicalDevice physDevice, const uint32_t queueFamily, const PlatformSurface surface );
bool isPresentationSupported( const VkPhysicalDevice physDevice, const PlatformSurface surface );

VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice physicalDevice, PlatformSurface surface );
VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice physicalDevice, PlatformSurface surface );

PlatformSwapchain initSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, PlatformSurface surface, VkSurfaceFormatKHR surfaceFormat, VkSurfaceCapabilitiesKHR capabilities, PlatformSwapchain oldSwapchain = VK_NULL_HANDLE );
void killSwapchain( VkDevice device, PlatformSwapchain swapchain );

std::vector<VkImage> getSwapchainImages( VkDevice device, PlatformSwapchain swapchain );
uint32_t getNextImageIndex( VkDevice device, VkQueue queue, PlatformSwapchain swapchain, VkSemaphore imageReadyS );
void present( VkQueue queue, PlatformSwapchain swapchain, uint32_t swapchainImageIndex, VkSemaphore renderDoneS );


void setSizeEventHandler( std::function<void(void)> newSizeEventHandler );
void setPaintEventHandler( std::function<void(void)> newPaintEventHandler );

void showWindow( PlatformWindow window );

// Implementation
//////////////////////////////////

bool nullHandler(){ return false; }

std::function<bool(void)> sizeEventHandler = nullHandler;

void setSizeEventHandler( std::function<bool(void)> newSizeEventHandler ){
	if( !newSizeEventHandler ) sizeEventHandler = nullHandler;
	sizeEventHandler = newSizeEventHandler;
}

std::function<void(void)> paintEventHandler = nullHandler;

void setPaintEventHandler( std::function<void(void)> newPaintEventHandler ){
	if( !newPaintEventHandler ) paintEventHandler = nullHandler;
	paintEventHandler = newPaintEventHandler;
}

void showWindow( PlatformWindow window ){
	ShowWindow( window.hWnd, SW_SHOW );
	SetForegroundWindow( window.hWnd );
}

bool hasSwapchain = false;

int messageLoop( PlatformWindow window ){
	UNREFERENCED_PARAMETER( window );

	MSG msg;
	BOOL ret = GetMessageW( &msg, NULL, 0, 0 );

	for( ; ret; ret = GetMessageW( &msg, NULL, 0, 0 ) ){
			TranslateMessage( &msg );
			DispatchMessageW( &msg ); //dispatch to wndProc; ignore return from wndProc
	}

	if( ret == -1 ) throw std::string( "Trouble getting message: " ) + std::to_string( GetLastError() );

	return static_cast<int>( msg.wParam );
}

TODO( "Could use IDXGISwapChain::SetFullscreenState()" )
void toggleFullscreen( HWND hWnd ){
	TODO( "All of this needs to be made a class... death to the static!" )
	static bool isFullscreen = false;
	static int wx = 100;
	static int wy = 100;
	static int ww = 800;
	static int wh = 800;

	DWORD style, exStyle;
	int x, y, width, height;

	if( !isFullscreen ){
		windowedStyle = GetWindowLongW( hWnd, GWL_STYLE );
		if( !windowedStyle && GetLastError() != ERROR_SUCCESS ) throw std::string( "Trouble setting window style: " ) + std::to_string( GetLastError() );

		windowedExStyle = GetWindowLongW( hWnd, GWL_EXSTYLE );
		if( !windowedExStyle && GetLastError() != ERROR_SUCCESS ) throw std::string( "Trouble setting window ex style: " ) + std::to_string( GetLastError() );

		const DWORD fullscreenStyle = ::windowedStyle & ~(WS_CAPTION | WS_THICKFRAME);
		const DWORD fullscreenExStyle = ::windowedExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);

		style = fullscreenStyle;
		exStyle = fullscreenExStyle;

		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof( MONITORINFO );
		{
			const auto succ = GetMonitorInfo(  MonitorFromWindow( hWnd, MONITOR_DEFAULTTOPRIMARY ), &monitorInfo  );
			if( !succ ) throw std::string( "Trouble getting window monitor info: " ) + std::to_string( GetLastError() );
		}
		x = monitorInfo.rcMonitor.left;
		y = monitorInfo.rcMonitor.top;
		width = monitorInfo.rcMonitor.right - x; assert( width >= 0 );
		height = monitorInfo.rcMonitor.bottom - y; assert( height >= 0 );

		RECT wndRect;
		{
			const auto succ = GetWindowRect( hWnd, &wndRect );
			if( !succ ) throw std::string( "Trouble getting window dimensions: " ) + std::to_string( GetLastError() );
		}
		wx = wndRect.left;
		wy = wndRect.top;
		ww = wndRect.right - wx;
		wh = wndRect.bottom - wy;
	}
	else {
		style = ::windowedStyle;
		exStyle = ::windowedExStyle;

		x = wx;
		y = wy;
		width = ww;
		height = wh;
	}

	isFullscreen = !isFullscreen;

	assert( GetLastError() == ERROR_SUCCESS );
	{
		const auto succ = SetWindowLongW( hWnd, GWL_STYLE, style );
		if( !succ && GetLastError() != ERROR_SUCCESS ) throw std::string( "Trouble setting window style: " ) + std::to_string( GetLastError() );
	}
	{
		const auto succ = SetWindowLongW( hWnd, GWL_EXSTYLE, exStyle );
		if( !succ && GetLastError() != ERROR_SUCCESS ) throw std::string( "Trouble setting window ex style: " ) + std::to_string( GetLastError() );
	}
	{
		const auto succ = SetWindowPos( hWnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOZORDER );
		if( !succ ) throw std::string( "Trouble resizing window to fullscreen: " ) + std::to_string( GetLastError() );
	}
}

LRESULT CALLBACK wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ){
	switch( uMsg ){
		case WM_CLOSE:
			PostQuitMessage( 0 );
			return 0;

		case WM_ERASEBKGND:
			//logger << "erase\n";
			//return DefWindowProc( hWnd, uMsg, wParam, lParam );
			return 0; // background will be cleared by Vulkan in WM_PAINT instead

		//case WM_NCCALCSIZE:
		//	//logger << "nccalcsize " << wParam << " " << lParam << endl;
		//	return DefWindowProc( hWnd, uMsg, wParam, lParam );

		case WM_SIZE:
			//logger << "size " << wParam << " " << LOWORD( lParam ) << "x" << HIWORD( lParam ) << std::endl;
			::currentWndSize = {LOWORD( lParam ), HIWORD( lParam )};
			hasSwapchain = sizeEventHandler();
			if( !hasSwapchain ) ValidateRect( hWnd, NULL ); // prevent WM_PAINT on minimized window
			return 0;

		case WM_PAINT: // sent after WM_SIZE -- react to this immediately to resize seamlessly
			//logger << "paint\n";
			paintEventHandler();
			//ValidateRect( hWnd, NULL ); // never validate so window always gets redrawn
			return 0;

		case WM_KEYDOWN:
			switch( wParam ){
			case VK_ESCAPE:
				PostQuitMessage( 0 );
				return 0;
			}
			return 0;

		case WM_SYSCOMMAND:
			switch (wParam) {
			case SC_KEYMENU:
				if (lParam == VK_RETURN) { // Alt-Enter without "no sysmenu hotkey exists" beep
					toggleFullscreen(hWnd);
					return 0;
				}
				else return DefWindowProc(hWnd, uMsg, wParam, lParam);
			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}

		default:
			//logger << "unknown " << to_string( uMsg ) << endl;
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
}


bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow ){
	UNREFERENCED_PARAMETER( instance );

	return vkGetPhysicalDeviceWin32PresentationSupportKHR( device, queueFamilyIndex ) == VK_TRUE;
}

ATOM classAtom = 0;
uint64_t classAtomRefCount = 0;

ATOM initWindowClass(){
	HINSTANCE hInstance = GetModuleHandleW( NULL );

	if( classAtom == 0 ){
		WNDCLASSEXW windowClass = {
			sizeof( WNDCLASSEXW ), // cbSize
			CS_OWNDC /*| CS_HREDRAW | CS_VREDRAW*/, // style -- some window behavior
			wndProc, // lpfnWndProc -- set event handler
			0, // cbClsExtra -- set 0 extra bytes after class
			0, // cbWndExtra -- set 0 extra bytes after class instance
			hInstance, // hInstance
			LoadIconW( NULL, IDI_APPLICATION ), // hIcon -- application icon
			LoadCursorW( NULL, IDC_ARROW ), // hCursor -- cursor inside
			NULL, //(HBRUSH)( COLOR_WINDOW + 1 ), // hbrBackground
			NULL, // lpszMenuName -- menu class name
			TEXT("vkwc"), // lpszClassName -- window class name/identificator
			LoadIconW( NULL, IDI_APPLICATION ) // hIconSm
		};

		// register window class
		classAtom = RegisterClassExW( &windowClass );
		if( !classAtom ){
			throw std::string( "Trouble registering window class: " ) + std::to_string( GetLastError() );
		}
	}

	++classAtomRefCount;
	return classAtom;
}

PlatformWindow initWindow( const std::string& name, uint32_t canvasWidth, uint32_t canvasHeight ){
	using std::string;
	using std::to_string;

	const string title = name + " -- DXGI Win32";
	const auto titleU16Size = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
	std::vector<wchar_t> titleU16(titleU16Size);
	const auto success = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, titleU16.data(), titleU16Size);

	HINSTANCE hInstance = GetModuleHandleW( NULL );

	const ATOM wndClassAtom = initWindowClass();

	// adjust size of window to contain given size canvas
	RECT windowRect = { 0, 0, static_cast<LONG>(canvasWidth), static_cast<LONG>(canvasHeight) };
	if(  !AdjustWindowRectEx( &windowRect, ::windowedStyle, FALSE, ::windowedExStyle )  ){
		throw string( "Trouble adjusting window size: " ) + to_string( GetLastError() );
	}

	// create window instance
	HWND hWnd =  CreateWindowExW(
		::windowedExStyle,
		MAKEINTATOM(wndClassAtom),
		titleU16.data(),
		::windowedStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if( !hWnd  ){
		throw string( "Trouble creating window instance: " ) + to_string( GetLastError() );
	}

	return { hInstance, hWnd };
}

void killWindow( PlatformWindow window ){
	using std::string;
	using std::to_string;

	if(  !DestroyWindow( window.hWnd )  ) throw string( "Trouble destroying window instance: " ) + to_string( GetLastError() );

	--classAtomRefCount;
	if( classAtomRefCount == 0 ){
		if(  !UnregisterClassW( MAKEINTATOM(classAtom), window.hInstance )  ){
			throw string( "Trouble unregistering window class: " ) + to_string( GetLastError() );
		}
	}
}

PlatformSurface initSurface( VkInstance /*instance*/, PlatformWindow window ){
	PlatformSurfaceImpl surface;
	surface.window = window;

	const PlatformSurface surfaceHandle = ::nextFreeSurfaceHandle;
	::platformSurfaces[surfaceHandle] = surface;
	++::nextFreeSurfaceHandle;

	return surfaceHandle;
}

void killSurface( VkInstance /*instance*/, PlatformSurface surface ){
	::platformSurfaces.erase( surface );
}

bool isPresentationSupported( const VkPhysicalDevice physDevice, const uint32_t /*queueFamily*/, const PlatformSurface /*surface*/ ){
	const auto deviceExtProps = enumerate<VkExtensionProperties>( physDevice );
	for( const auto de : ::requiredDeviceExtensions ){
		if(
			std::find_if( deviceExtProps.begin(), deviceExtProps.end(), [&de]( const VkExtensionProperties& prop ){ return std::strcmp(prop.extensionName, de) == 0; } )
			==
			deviceExtProps.end()
		){
			return false;
		}
	}

	VkExternalImageFormatProperties extImgProps = {VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES, nullptr, {0xCDCDCDCD,0xCDCDCDCD,0xCDCDCDCD}};
	VkImageFormatProperties2 imgFmtProps = {VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2, &extImgProps};

	VkPhysicalDeviceExternalImageFormatInfo extImgFormatInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO, nullptr, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT};
	VkPhysicalDeviceImageFormatInfo2 imgFormatInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2, &extImgFormatInfo, ::extImgFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0};
	{const VkResult errorCode = vkGetPhysicalDeviceImageFormatProperties2KHR( physDevice, &imgFormatInfo, &imgFmtProps ); RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceImageFormatProperties2KHR" );}

	TODO( "Workaround for AMD driver not reporting values" );
	if( extImgProps.externalMemoryProperties.externalMemoryFeatures == 0xCDCDCDCD ) extImgProps.externalMemoryProperties.externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;

	if( !(extImgProps.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) ) return false;
	//extImgProps.externalMemoryProperties.exportFromImportedHandleTypes
	//extImgProps.externalMemoryProperties.compatibleHandleTypes

	TODO( "Might want to do some extra queries here? External Fences\\Semaphores support?" );
	return true;
}

VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice /*physicalDevice*/, PlatformSurface /*surface*/ ){
	const VkSurfaceFormatKHR surfaceFmt = {::extImgFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	return surfaceFmt;
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice /*physicalDevice*/, PlatformSurface /*surface*/ ){
	TODO( "Mostly bs values for now" );
	VkSurfaceCapabilitiesKHR capabilities;
	capabilities.minImageCount = 2; // DXGI_MIN_SWAP_CHAIN_BUFFERS for flip
	capabilities.maxImageCount = DXGI_MAX_SWAP_CHAIN_BUFFERS;
	capabilities.currentExtent = currentWndSize;
	capabilities.minImageExtent = currentWndSize;
	capabilities.maxImageExtent = currentWndSize;
	capabilities.maxImageArrayLayers = 1;
	capabilities.supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	capabilities.currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	capabilities.supportedCompositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	capabilities.supportedUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	return capabilities;
}

void ChB( const BOOL result ){
	if( result == FALSE ){
		throw "Non-zero BOOL result!";
	}
}

void ChHR( const HRESULT hr ){
	if( hr != S_OK ){
		throw "Non-OK HRESULT!";
	}
}

PlatformSwapchain initSwapchain(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	PlatformSurface surface,
	VkSurfaceFormatKHR surfaceFormat,
	VkSurfaceCapabilitiesKHR capabilities,
	uint32_t /*graphicsQueueFamily*/,
	uint32_t /*presentQueueFamily*/,
	PlatformSwapchain oldSwapchain
){
	TODO( "Use presentationMode" );

/*
	TODO( "Perhaps not really necessary to have VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR" )
	if(  !( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR )  ){
		throw "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR not supported!";
	}
*/

	// for all modes having at least two Images can be beneficial
	TODO( "Or is it minImageCount + 1 ? Gotta think this through." )
	uint32_t minImageCount = std::max<uint32_t>( 2, capabilities.minImageCount );
	if( capabilities.maxImageCount ) minImageCount = std::min<uint32_t>( minImageCount, capabilities.maxImageCount );

	const UINT swapchainFlags = 0;

	DXGI_FORMAT format;
	if( surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM ) format = DXGI_FORMAT_B8G8R8A8_UNORM;
	else throw "Unknown swapchain format!";

	const auto& newSurfaceImpl = ::platformSurfaces.at( surface );

	PlatformSwapchain newSwapchain;

	if( oldSwapchain && newSurfaceImpl.window.hWnd == ::platformSwapchains.at( oldSwapchain ).surface.window.hWnd ){ // mostly everything allowed to change with ResizeBuffers
		auto& oldSwapImpl = ::platformSwapchains.at( oldSwapchain );
		oldSwapImpl.imageCount = minImageCount;

		for( const auto im : oldSwapImpl.importedMemories ) vkFreeMemory( device, im, nullptr );
		oldSwapImpl.importedMemories.clear();
		for( const auto sh : oldSwapImpl.sharedHandles ) ChB( CloseHandle( sh ) );
		oldSwapImpl.sharedHandles.clear();
		for( const auto img : oldSwapImpl.swapchainImages ) vkDestroyImage( device, img, nullptr );
		oldSwapImpl.swapchainImages.clear();

		ChHR(  oldSwapImpl.dxgiSwapchain->ResizeBuffers( oldSwapImpl.imageCount, capabilities.currentExtent.width, capabilities.currentExtent.height, format, swapchainFlags)  );

		newSwapchain = oldSwapchain;
	}
	else{
		PlatformSwapchainImpl swapchain;
		swapchain.surface = ::platformSurfaces[surface];
		swapchain.physicalDevice = physicalDevice;
		swapchain.device = device;

		swapchain.imageCount = minImageCount;

#ifdef NDEBUG
		UINT createFacFlags = 0;
#else
		const UINT createFacFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		ChHR(  CreateDXGIFactory2( createFacFlags, IID_PPV_ARGS(&swapchain.dxgiFac) )  );

#ifndef NDEBUG
		ChHR(  DXGIGetDebugInterface1( 0, IID_PPV_ARGS(&swapchain.dxgiDebug) )  );
		ChHR(  DXGIGetDebugInterface1( 0, IID_PPV_ARGS(&swapchain.dxgiDebugQueue) )  );

		swapchain.dxgiDebug->EnableLeakTrackingForThread();
		DXGI_INFO_QUEUE_FILTER dxgiFilter = {};
		std::vector<DXGI_INFO_QUEUE_MESSAGE_SEVERITY> dxgiDeniedSeverities = {DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO};
		dxgiFilter.DenyList.NumSeverities = static_cast<UINT>( dxgiDeniedSeverities.size() );
		dxgiFilter.DenyList.pSeverityList = dxgiDeniedSeverities.data();
		ChHR(  swapchain.dxgiDebugQueue->PushRetrievalFilter( DXGI_DEBUG_ALL, &dxgiFilter )  );
		ChHR(  swapchain.dxgiDebugQueue->PushStorageFilter( DXGI_DEBUG_ALL, &dxgiFilter )  );
		TODO( "For some reason DXGI_DEBUG_ALL returns E_INAVALIDARG." );
		ChHR(  swapchain.dxgiDebugQueue->SetMessageCountLimit( DXGI_DEBUG_DXGI, static_cast<UINT64>(-1) )  );
		ChHR(  swapchain.dxgiDebugQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE )  );
		ChHR(  swapchain.dxgiDebugQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE )  );
		ChHR(  swapchain.dxgiDebugQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, TRUE )  );

		TODO( "DX12 Debug actually makes the window unresponsive" );
		//ChHR(  D3D12GetDebugInterface( IID_PPV_ARGS(&swapchain.dx12Debug) )  );
		//swapchain.dx12Debug->EnableDebugLayer();
		//swapchain.dx12Debug->SetEnableGPUBasedValidation( TRUE );
#endif

		VkPhysicalDeviceIDPropertiesKHR pdidProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR, nullptr };
		VkPhysicalDeviceProperties2KHR pdProps2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR, &pdidProps };
		vkGetPhysicalDeviceProperties2KHR( physicalDevice, &pdProps2 );
		if( !pdidProps.deviceLUIDValid ) throw "LUID not valid!";


		LUID* pluid = reinterpret_cast<LUID*>( &pdidProps.deviceLUID );
		ChHR(  swapchain.dxgiFac->EnumAdapterByLuid(  *pluid, IID_PPV_ARGS( &swapchain.dxgiAdapter )  )  );
		ChHR(  D3D12CreateDevice( swapchain.dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&swapchain.dxDevice) )  ); TODO( "Check feature level" );

#ifndef NDEBUG
		//ChHR(  swapchain.dxDevice.As( &swapchain.dx12DebugQueue )  );

		//D3D12_INFO_QUEUE_FILTER dx12Filter = {};
		//std::vector<D3D12_MESSAGE_SEVERITY> dx12DeniedSeverities = {D3D12_MESSAGE_SEVERITY_MESSAGE, D3D12_MESSAGE_SEVERITY_INFO};
		//dx12Filter.DenyList.NumSeverities = static_cast<UINT>( dx12DeniedSeverities.size() );
		//dx12Filter.DenyList.pSeverityList = dx12DeniedSeverities.data();
		//ChHR(  swapchain.dx12DebugQueue->PushRetrievalFilter( &dx12Filter )  );
		//ChHR(  swapchain.dx12DebugQueue->PushStorageFilter( &dx12Filter )  );
		//TODO( "For some reason DXGI_DEBUG_ALL returns E_INAVALIDARG." );
		//ChHR(  swapchain.dx12DebugQueue->SetMessageCountLimit( static_cast<UINT64>(-1) )  );
		//ChHR(  swapchain.dx12DebugQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE )  );
		//ChHR(  swapchain.dx12DebugQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, TRUE )  );
		//ChHR(  swapchain.dx12DebugQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, TRUE )  );
#endif

		const auto nodeCount = swapchain.dxDevice->GetNodeCount();
		const UINT nodeMask = nodeCount <= 1 ? 0 : pdidProps.deviceNodeMask;

		const D3D12_COMMAND_QUEUE_DESC dxQDesc = {
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAG_NONE,
			nodeMask
		};
		ChHR(  swapchain.dxDevice->CreateCommandQueue( &dxQDesc, IID_PPV_ARGS(&swapchain.dxCommandQueue) )  );

		TODO( "It should have DXGI_USAGE_SHARED, but that is invalid and crashes. And it works without it anyway" )
		const DXGI_USAGE usage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		const DXGI_SWAP_EFFECT presentMode = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // Vulkan guarantees original content for some reason

		const DXGI_SWAP_CHAIN_DESC1 swapchainDesc{
			capabilities.currentExtent.width,
			capabilities.currentExtent.height,
			format,
			FALSE, // Stereo
			{ 1, 0 }, // Samples
			usage,
			minImageCount,
			DXGI_SCALING_NONE,
			presentMode,
			DXGI_ALPHA_MODE_IGNORE,
			swapchainFlags
		};

		ComPtr<IDXGISwapChain1> swapchain1;
		ChHR(  swapchain.dxgiFac->CreateSwapChainForHwnd( swapchain.dxCommandQueue.Get(), swapchain.surface.window.hWnd, &swapchainDesc, nullptr, nullptr, &swapchain1 )  );
		ChHR(  swapchain1.As( &swapchain.dxgiSwapchain )  );

		ChHR(  swapchain.dxgiFac->MakeWindowAssociation( swapchain.surface.window.hWnd, DXGI_MWA_NO_ALT_ENTER )  );

		newSwapchain = ::nextFreeSwapchainHandle;
		::platformSwapchains[newSwapchain] = swapchain;
		::nextFreeSwapchainHandle++;
	}

	return newSwapchain;
}

void killSwapchain( VkDevice device, PlatformSwapchain swapchain ){
	if( swapchain ){
		const auto& swapImpl = ::platformSwapchains.at(swapchain);
		for( const auto im : swapImpl.importedMemories ) vkFreeMemory( device, im, nullptr );
		for( const auto sh : swapImpl.sharedHandles ) ChB( CloseHandle( sh ) );
		for( const auto img : swapImpl.swapchainImages ) vkDestroyImage( device, img, nullptr );

		::platformSwapchains.erase( swapchain );
	}
}

std::vector<VkImage> getSwapchainImages( VkDevice device, PlatformSwapchain swapchain ){
	auto& swapImpl = ::platformSwapchains.at(swapchain);

	if( swapImpl.swapchainImages.empty() ){
		std::vector< ComPtr<ID3D12Resource> > dxImages( swapImpl.imageCount );
		swapImpl.swapchainImages.resize( dxImages.size() );
		swapImpl.importedMemories.resize( dxImages.size() );
		swapImpl.sharedHandles.resize( dxImages.size() );

		for( UINT i = 0; i < dxImages.size(); ++i ){
			ChHR(  swapImpl.dxgiSwapchain->GetBuffer( i, IID_PPV_ARGS(&dxImages[i]) )  );
		}

		for( size_t i = 0; i < dxImages.size(); ++i ){
			const auto& dxImage = dxImages[i];
			const auto dxImageDesc = dxImage->GetDesc();

			D3D12_HEAP_PROPERTIES dxImageHeap;
			D3D12_HEAP_FLAGS dxImageHeapFlags;
			ChHR( dxImage->GetHeapProperties( &dxImageHeap, &dxImageHeapFlags ) );

			// just to be sure check if images match our assumptions
			//DXGI_USAGE usage;
			if( dxImageDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ) throw "Weird DXGI image dimensionality";
			if( dxImageDesc.DepthOrArraySize != 1 ) throw "Weird DXGI image array count";
			if( dxImageDesc.MipLevels != 1 ) throw "Weird DXGI image mip level count";
			if( dxImageDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM ) throw "Weird DXGI image format";
			if( dxImageDesc.SampleDesc.Count != 1 ) throw "Weird DXGI image sample count";

			VkExternalMemoryImageCreateInfoKHR eii = {
				VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR,
				nullptr, // pNext
				VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT_KHR
			};

			assert( dxImageDesc.Width <= UINT32_MAX );

			const VkImageCreateInfo ii = {
				VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				&eii,
				0, // flags
				VK_IMAGE_TYPE_2D,
				::extImgFormat,
				{static_cast<uint32_t>(dxImageDesc.Width), static_cast<uint32_t>(dxImageDesc.Height), 1},
				1, 1, VK_SAMPLE_COUNT_1_BIT, // mip, array, samples
				VK_IMAGE_TILING_OPTIMAL, // assumably
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				0, nullptr, // queue families share
				VK_IMAGE_LAYOUT_UNDEFINED // per Vk VU
			};

			{const VkResult errorCode = vkCreateImage( device, &ii, nullptr, &swapImpl.swapchainImages[i] ); RESULT_HANDLER( errorCode, "vkCreateImage" );}

			TODO("Not sure why I added this optional value. Was this a workaround?")
			std::wstring sharedHandleName = std::wstring(L"Local\\SomeBullshitNameIDontNeedAnyway") + std::to_wstring(i);
			TODO("I am pretty sure DX Swapchain is not considered SHARED, but it seems to work anyway.")
			ChHR( swapImpl.dxDevice->CreateSharedHandle( dxImage.Get(), NULL, GENERIC_ALL, sharedHandleName.data(), &swapImpl.sharedHandles[i] ) );

			VkMemoryWin32HandlePropertiesKHR w32MemProps{ VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR, nullptr, 0xcdcdcdcd };
			{const VkResult errorCode = vkGetMemoryWin32HandlePropertiesKHR( device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT, swapImpl.sharedHandles[i], &w32MemProps ); RESULT_HANDLER( errorCode, "vkGetMemoryWin32HandlePropertiesKHR" );}

			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements( device, swapImpl.swapchainImages[i], &memReq );

			TODO( "Workaround for AMD driver." );
			if( w32MemProps.memoryTypeBits == 0xcdcdcdcd ) w32MemProps.memoryTypeBits = memReq.memoryTypeBits;
			else w32MemProps.memoryTypeBits &= memReq.memoryTypeBits; // assumably must satisfy both?

			VkPhysicalDeviceMemoryProperties memProps;
			vkGetPhysicalDeviceMemoryProperties( swapImpl.physicalDevice, &memProps );


			int memTypeIndex = -1;
			for( uint32_t im = 0; im < memProps.memoryTypeCount; ++im ){
				const uint32_t current_bit = 0x1 << im;
				if( w32MemProps.memoryTypeBits == current_bit ){
					if( memProps.memoryTypes[im].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) memTypeIndex = im;
					break;
				}
			}

			if( memTypeIndex < 0 ) throw "Device local import memory not found!";

			// DX12 Resource has to be dedicated per Vk spec
			const VkMemoryDedicatedAllocateInfoKHR dii = {
				VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
				nullptr, // pNext
				swapImpl.swapchainImages[i],
				VK_NULL_HANDLE // buffer
			};

			const VkImportMemoryWin32HandleInfoKHR imi{
				VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
				&dii,
				VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT_KHR,
				swapImpl.sharedHandles[i],
				nullptr // handle name -- redundant
			};

			const VkMemoryAllocateInfo mi{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				&imi,
				memReq.size,
				static_cast<uint32_t>( memTypeIndex )
			};

			{const VkResult errorCode = vkAllocateMemory( device, &mi, nullptr, &swapImpl.importedMemories[i] ); RESULT_HANDLER( errorCode, "vkAllocateMemory" );}
			{const VkResult errorCode = vkBindImageMemory( device, swapImpl.swapchainImages[i], swapImpl.importedMemories[i], 0 ); RESULT_HANDLER( errorCode, "vkBindImageMemory" );}
		}
	}

	return swapImpl.swapchainImages;
}

TODO( "Gets a queue on top of vkAcquireNextImageKHR, in order to validly signal that semaphore." );
TODO( "Does not really allow to acquire multiple images like in Vulkan." );
uint32_t getNextImageIndex( VkDevice /*device*/, VkQueue queue, PlatformSwapchain swapchain, VkSemaphore imageReadyS ){
	const auto& swapImpl = ::platformSwapchains.at(swapchain);

	TODO( "Present assumably blocks, so no sync necessary. Do better sync with DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT?" );
	const VkSubmitInfo submit{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr, // pNext
		0, nullptr, nullptr, // wait semaphores
		0, nullptr, // cmdbuffers
		1, &imageReadyS
	};
	{const VkResult errorCode = vkQueueSubmit( queue, 1, &submit, VK_NULL_HANDLE ); RESULT_HANDLER( errorCode, "vkQueueSubmit" );}

	const uint32_t nextImage = swapImpl.dxgiSwapchain->GetCurrentBackBufferIndex();

	return nextImage;
}

TODO( "Does not really use swapchainImageIndex, because DX does not work with multiple images." );
void present( VkQueue queue, PlatformSwapchain swapchain, uint32_t swapchainImageIndex, VkSemaphore renderDoneS ){
	const auto& swapImpl = ::platformSwapchains.at(swapchain);
	assert( swapchainImageIndex == swapImpl.dxgiSwapchain->GetCurrentBackBufferIndex() );

	const VkFenceCreateInfo fci{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr, // pNext
		0 // flags
	};

	VkFence fence;
	{const VkResult errorCode = vkCreateFence( swapImpl.device, &fci, nullptr, &fence );  RESULT_HANDLER( errorCode, "vkCreateFence" );}

	const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	const VkSubmitInfo submit{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr, // pNext
		1, &renderDoneS, &waitStage,
		0, nullptr, // cmdbuffers
		0, nullptr // signal semaphores
	};
	{const VkResult errorCode = vkQueueSubmit( queue, 1, &submit, fence ); RESULT_HANDLER( errorCode, "vkQueueSubmit" );}
	{const VkResult errorCode = vkWaitForFences( swapImpl.device, 1, &fence, VK_TRUE, UINT64_MAX ); RESULT_HANDLER( errorCode, "vkWaitForFences" );}

	vkDestroyFence( swapImpl.device, fence, nullptr );

	const UINT vsyncs = 1;
	const UINT presentFlags = 0; // assumably
	const DXGI_PRESENT_PARAMETERS presentParams = {};
	TODO( "Should handle alternative return codes" );
	TODO( "There are some artifacts, which suggest *something* has gone horribly wrong..." )
	ChHR( swapImpl.dxgiSwapchain->Present1( vsyncs, presentFlags, &presentParams ) );
}

#endif //COMMON_DXGI_WSI_H