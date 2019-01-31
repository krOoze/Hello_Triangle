// win32 platform dependent WSI handling and event loop

#ifndef COMMON_WIN32_WSI_H
#define COMMON_WIN32_WSI_H

#include <functional>
#include <string>
#include <vector>

#include <Windows.h>
#include <vulkan/vulkan.h>

#include "CompilerMessages.h"
#include "ErrorHandling.h"


TODO( "Easier to use, but might prevent platform co-existence. Could be namespaced. Make all of this a class?" )
struct PlatformWindow{ HINSTANCE hInstance; HWND hWnd; };

std::string getPlatformSurfaceExtensionName(){ return VK_KHR_WIN32_SURFACE_EXTENSION_NAME; };

int messageLoop( PlatformWindow window );

bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow window );

PlatformWindow initWindow( const std::string& name, uint32_t canvasWidth, uint32_t canvasHeight );
void killWindow( PlatformWindow window );

VkSurfaceKHR initSurface( VkInstance instance, PlatformWindow window );
// killSurface() is not platform dependent

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

	DWORD windowedStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD windowedExStyle = WS_EX_OVERLAPPEDWINDOW;

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
			default:
				return DefWindowProc( hWnd, uMsg, wParam, lParam );
			}

		case WM_SYSCOMMAND:
			switch( wParam ){
			case SC_KEYMENU:
				if( lParam == VK_RETURN ){ // Alt-Enter without "no sysmenu hotkey exists" beep
					toggleFullscreen( hWnd );
					return 0;
				}
				else return DefWindowProc( hWnd, uMsg, wParam, lParam );
			default:
				return DefWindowProc( hWnd, uMsg, wParam, lParam );
			}

		default:
			//logger << "unknown " << std::hex << std::showbase << uMsg  << std::endl;
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

	const string title = name + " -- Win32";
	const auto titleU16Size = MultiByteToWideChar( CP_UTF8, 0, title.c_str(), -1, nullptr, 0 );
	std::vector<wchar_t> titleU16( titleU16Size );
	const auto success = MultiByteToWideChar( CP_UTF8, 0, title.c_str(), -1, titleU16.data(), titleU16Size );

	HINSTANCE hInstance = GetModuleHandleW( NULL );

	ATOM wndClassAtom = initWindowClass();

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

VkSurfaceKHR initSurface( VkInstance instance, PlatformWindow window ){
	VkWin32SurfaceCreateInfoKHR surfaceInfo{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		window.hInstance,
		window.hWnd
	};


	VkSurfaceKHR surface;
	VkResult errorCode = vkCreateWin32SurfaceKHR( instance, &surfaceInfo, nullptr, &surface ); RESULT_HANDLER( errorCode, "vkCreateWin32SurfaceKHR" );

	return surface;
}


#endif //COMMON_WIN32_WSI_H