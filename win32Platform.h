// win32 platform dependent WSI handling and event loop

#ifndef COMMON_WIN32_WSI_H
#define COMMON_WIN32_WSI_H

#include <Windows.h>
#include <vulkan/vulkan.h>

#include "CompilerMessages.h"


TODO( "Easier to use, but might prevent platform co-existence" )
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
struct PlatformWindow{ HINSTANCE hInstance; ATOM wndClass; HWND hWnd; };

int messageLoop( bool& quit );
LRESULT CALLBACK wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

bool presentationSupport( VkPhysicalDevice device, uint32_t queueFamilyIndex );

PlatformWindow initWindow( int canvasWidth, int canvasHeight );
void killWindow( PlatformWindow window );

VkSurfaceKHR initSurface( VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t queueFamily, PlatformWindow window );
// killSurface() is not platform dependent



// In case of non console subsystem just relay to main()
int main();
int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow ){
	UNREFERENCED_PARAMETER( hInstance );
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );
	UNREFERENCED_PARAMETER( nCmdShow );

	return main();
}

// Implementation
//////////////////////////////////

int messageLoop( bool& quit ){
	MSG msg;

	while(  PeekMessageW( &msg, NULL, 0, 0, PM_REMOVE )  ){ // peek does not block on empty queue!
		if( msg.message != WM_QUIT ){
			TranslateMessage( &msg );
			DispatchMessageW( &msg ); //dispatch to wndProc; ignore return from wndProc
		}
		else{
			quit = true;
			return static_cast<int>( msg.wParam );
		}
	}

	return EXIT_SUCCESS;
}

LRESULT CALLBACK wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ){
	UNREFERENCED_PARAMETER( lParam );

	switch( uMsg ){
	case WM_CLOSE:
		PostQuitMessage( 0 );
		return 0;

	case WM_PAINT:
		//ValidateRect( window, NULL );
		return DefWindowProc( hWnd, uMsg, wParam, lParam );

	case WM_KEYDOWN:
		switch( wParam ){
		case VK_ESCAPE:
			PostQuitMessage( 0 );
			return 0;
		}
		return 0;

	default:
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
}


bool presentationSupport( VkPhysicalDevice device, uint32_t queueFamilyIndex ){
	return vkGetPhysicalDeviceWin32PresentationSupportKHR( device, queueFamilyIndex ) == VK_TRUE;
}

PlatformWindow initWindow( int canvasWidth, int canvasHeight ){
	HINSTANCE hInstance = GetModuleHandleW( NULL );

	TODO( "There should be only one wnd class made, but this will do for now -- could use overal generalization" )
	static unsigned uniqueCounter = 0;
	if( uniqueCounter == 1000 ) throw "What the is wrong with you... I mean: too many window class registrations made!";
	std::wstring className =  L"vkwc" + std::to_wstring( uniqueCounter++ );
	WNDCLASSEXW windowClass = {
		sizeof( WNDCLASSEXW ), // cbSize
		CS_OWNDC | CS_HREDRAW | CS_VREDRAW, // style - some window behavior
		wndProc, // lpfnWndProc - set event handler
		0, // cbClsExtra - set 0 extra bytes after class
		0, // cbWndExtra - set 0 extra bytes after class instance
		hInstance, // hInstance
		LoadIconW( NULL, IDI_APPLICATION ), // hIcon - application icon
		LoadCursorW( NULL, IDC_ARROW ), // hCursor - cursor inside
		(HBRUSH)COLOR_WINDOW, // hbrBackground - no background prepainting
		NULL, // lpszMenuName - menu class name
		className.c_str(), // lpszClassName - window class name/identificator
		LoadIconW( NULL, IDI_APPLICATION /*IDI_WINLOGO*/ ) // hIconSm
	};

	// register window class
	ATOM classAtom = RegisterClassExW( &windowClass );
	if( !classAtom ){
		throw string( "Trouble registering window class: " ) + to_string( GetLastError() );
	}

	// adjust size of window to contain given size canvas
	DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;
	RECT windowRect = { 0, 0, canvasWidth, canvasHeight };
	if(  !AdjustWindowRectEx( &windowRect, style, FALSE, exStyle )  ){
		throw string( "Trouble adjusting window size: " ) + to_string( GetLastError() );
	}

	// create window instance
	HWND hWnd =  CreateWindowExW(
		exStyle,
		MAKEINTATOM(classAtom),
		TEXT("Hello Vulkan Triangle"),
		style,
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

	ShowWindow( hWnd, SW_SHOW );
	SetForegroundWindow( hWnd );

	return { hInstance, classAtom, hWnd };
}

void killWindow( PlatformWindow window ){
	if(  !DestroyWindow( window.hWnd )  ) throw string( "Trouble destroying window instance: " ) + to_string( GetLastError() );
	if(  !UnregisterClassW( MAKEINTATOM(window.wndClass), window.hInstance )  ){
		throw string( "Trouble unregistering window class: " ) + to_string( GetLastError() );
	}
}

VkSurfaceKHR initSurface( VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t queueFamily, PlatformWindow window ){
	VkWin32SurfaceCreateInfoKHR surfaceInfo{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		window.hInstance,
		window.hWnd
	};


	VkSurfaceKHR surface;
	VkResult errorCode = vkCreateWin32SurfaceKHR( instance, &surfaceInfo, nullptr, &surface ); RESULT_HANDLER( errorCode, "vkCreateWin32SurfaceKHR" );

	// validate WSI support
	VkBool32 supported;
	errorCode = vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, queueFamily, surface, &supported ); RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfaceSupportKHR" );
	if( !supported ) throw "Selected queue family of physical device can't present to the surface!";

	return surface;
}


#endif //COMMON_WIN32_WSI_H