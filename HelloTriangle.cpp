// Vulkan hello world triangle rendering demo


// Environment
//////////////////
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <string>
using std::string;
using std::to_string;

#include <exception>
using std::exception;

#include <stdexcept>
using std::runtime_error;

#include<cstring>

#include <fstream>
using std::ifstream;

#include <iterator>
using std::istreambuf_iterator;

#include <chrono>
using std::chrono::steady_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

#include <cmath>

#ifdef _WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#elif __CYGWIN__
	#define VK_USE_PLATFORM_WIN32_KHR
#else
	#error "Unsupported platform"
#endif
#include "VulkanEnvironment.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
	#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

// Config
///////////////////////

// layers and debug
#ifdef _DEBUG
constexpr bool debugVulkan = true;
constexpr VkDebugReportFlagsEXT debugAmount =
	/*VK_DEBUG_REPORT_INFORMATION_BIT_EXT |*/
	VK_DEBUG_REPORT_WARNING_BIT_EXT |
	VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
	VK_DEBUG_REPORT_ERROR_BIT_EXT /*|
	VK_DEBUG_REPORT_DEBUG_BIT_EXT*/;
#else
constexpr bool debugVulkan = false;
constexpr VkDebugReportFlagsEXT debugAmount = 0;
#endif

// window and swapchain
constexpr int windowWidth = 800;
constexpr int windowHeight = 800;

constexpr VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

// pipeline settings
constexpr VkClearValue clearColor = { {0.1f, 0.1f, 0.1f, 1.0f} };

const char* vertexShaderFilename = "triangle.vert.spv";
const char* fragmentShaderFilename = "triangle.frag.spv";

// Error handling
//////////////////////

struct VulkanResultException{
	const char* file;
	unsigned line;
	const char* func;
	const char* source;
	VkResult result;

	VulkanResultException( const char* file, unsigned line, const char* func, const char* source, VkResult result )
	: file( file ), line( line ), func( func ), source( source ), result( result ){}
};

#define RESULT_HANDLER( errorCode, source )  if( errorCode ) throw VulkanResultException( __FILE__, __LINE__, __func__, source, errorCode )
#define RESULT_HANDLER_EX( cond, errorCode, source )  if( cond ) throw VulkanResultException( __FILE__, __LINE__, __func__, source, errorCode )

const char* to_string( VkResult r );

// needed stuff -- forward declarations
///////////////////////////////
struct Vertex { float position[2]; float color[3]; };


VkInstance initInstance( const vector<const char*> layers = {}, const vector<const char*> extensions = {} );
void killInstance( VkInstance instance );

void loadVulkanExtensions( VkInstance instance );

VkDebugReportCallbackEXT initDebug( VkInstance instance );
void killDebug( VkInstance instance, VkDebugReportCallbackEXT debug );


VkPhysicalDevice getPhysicalDevice( VkInstance instance ); // destroyed with instance
VkPhysicalDeviceProperties getPhysicalDeviceProperties( VkPhysicalDevice physicalDevice );
VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties( VkPhysicalDevice physicalDevice );

uint32_t getQueueFamily( VkPhysicalDevice physDevice );

VkDevice initDevice(
	VkPhysicalDevice physDevice,
	const VkPhysicalDeviceFeatures& features,
	uint32_t queueFamilyIndex,
	vector<const char*> layers = {},
	vector<const char*> extensions = {}
);
VkDevice initDevice( // for all features
	VkPhysicalDevice physDevice,
	uint32_t queueFamilyIndex,
	vector<const char*> layers = {},
	vector<const char*> extensions = {}
);
void killDevice( VkDevice device );

VkQueue getQueue( VkDevice device, uint32_t queueFamily, uint32_t queueIndex );


enum class ResourceType{ Buffer, Image };

template< ResourceType resourceType, class T >
VkDeviceMemory initMemory(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	T resource,
	VkMemoryPropertyFlags requiredFlags,
	VkMemoryPropertyFlags desiredFlags = 0
);
void setMemoryData( VkDevice device, VkDeviceMemory memory, void* begin, size_t size );
void killMemory( VkDevice device, VkDeviceMemory memory );

VkBuffer initBuffer( VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage );
void killBuffer( VkDevice device, VkBuffer buffer );

VkImage initImage(
	VkDevice device,
	VkFormat format,
	uint32_t width, uint32_t height,
	VkSampleCountFlagBits samples,
	VkImageUsageFlags usage
);
void killImage( VkDevice device, VkImage image );

VkImageView initImageView( VkDevice device, VkImage image, VkFormat format );
void killImageView( VkDevice device, VkImageView imageView );


#ifdef VK_USE_PLATFORM_WIN32_KHR
struct PlatformWindow{ HINSTANCE hInstance; ATOM wndClass; HWND hWnd; };
#endif

PlatformWindow initWindow( int canvasWidth, int canvasHeight );
void killWindow( PlatformWindow window );

VkSurfaceKHR initSurface( VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t queueFamily, PlatformWindow window );
void killSurface( VkInstance instance, VkSurfaceKHR surface );

VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );
VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );

VkSwapchainKHR initSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat );
void killSwapchain( VkDevice device, VkSwapchainKHR swapchain );

vector<VkImage> getSwapchainImages( VkDevice device, VkSwapchainKHR swapchain );

uint32_t getNextImageIndex( VkDevice device, VkSwapchainKHR swapchain, VkSemaphore imageReadyS );

vector<VkImageView> initSwapchainImageViews( VkDevice device, vector<VkImage> images, VkFormat format );
void killSwapchainImageViews( VkDevice device, vector<VkImageView> imageViews );


VkRenderPass initRenderPass( VkDevice device, VkSurfaceFormatKHR surfaceFormat );
void killRenderPass( VkDevice device, VkRenderPass renderPass );

vector<VkFramebuffer> initFramebuffers(
	VkDevice device,
	VkRenderPass renderPass,
	vector<VkImageView> imageViews,
	uint32_t width, uint32_t height
);
void killFramebuffers( VkDevice device, vector<VkFramebuffer> framebuffers );


VkShaderModule initShaderModule( VkDevice device, string filename );
void killShaderModule( VkDevice device, VkShaderModule shaderModule );

VkPipelineLayout initPipelineLayout( VkDevice device );
void killPipelineLayout( VkDevice device, VkPipelineLayout pipelineLayout );

VkPipeline initPipeline(
	VkDevice device,
	VkPhysicalDeviceLimits limits,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass,
	VkShaderModule vertexShader,
	VkShaderModule fragmentShader,
	const uint32_t vertexBufferBinding
);
void killPipeline( VkDevice device, VkPipeline pipeline );


void setVertexData( VkDevice device, VkDeviceMemory memory, vector<Vertex> vertices );

VkSemaphore initSemaphore( VkDevice device );
void killSemaphore( VkDevice device, VkSemaphore semaphore );

VkCommandPool initCommandPool( VkDevice device, const uint32_t queueFamily );
void killCommandPool( VkDevice device, VkCommandPool commandPool );

vector<VkCommandBuffer> acquireCommandBuffers( VkDevice device, VkCommandPool commandPool, uint32_t count );
void beginCommandBuffer( VkCommandBuffer commandBuffer );
void endCommandBuffer( VkCommandBuffer commandBuffer );

void recordBeginRenderPass(
	VkCommandBuffer commandBuffer,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	VkClearValue clearValue,
	uint32_t width, uint32_t height
);
void recordEndRenderPass( VkCommandBuffer commandBuffer );

void recordBindPipeline( VkCommandBuffer commandBuffer, VkPipeline pipeline );
void recordBindVertexBuffer( VkCommandBuffer commandBuffer, const uint32_t vertexBufferBinding, VkBuffer vertexBuffer );

void recordSetViewport( VkCommandBuffer commandBuffer, uint32_t width, uint32_t height );
void recordSetScissor( VkCommandBuffer commandBuffer, uint32_t width, uint32_t height );

void recordDraw( VkCommandBuffer commandBuffer, const vector<Vertex> vertices );

void submitToQueue( VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore imageReadyS, VkSemaphore renderDoneS );
void present( VkQueue queue, VkSwapchainKHR swapchain, uint32_t swapchainImageIndex, VkSemaphore renderDoneS );

int messageLoop( bool& quit );

// main()!
////////////////////////

int main() try{
	const uint32_t vertexBufferBinding = 0;

	const float triangleSize = 1.6f;
	vector<Vertex> triangle = {
		{ /*rb*/ {  0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize }, /*R*/{ 1.0f, 0.0f, 0.0f }  },
		{ /* t*/ {                 0.0f, -sqrtf( 3.0f ) * 0.25f * triangleSize }, /*G*/{ 0.0f, 1.0f, 0.0f }  },
		{ /*lb*/ { -0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize }, /*B*/{ 0.0f, 0.0f, 1.0f }  }
	};

	vector<const char*> layers;
	if( ::debugVulkan ) layers.push_back( "VK_LAYER_LUNARG_standard_validation" );

	VkInstance instance = initInstance(
		layers,
		{VK_KHR_SURFACE_EXTENSION_NAME, PLATFORM_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME}
	);
	loadVulkanExtensions( instance );
	VkDebugReportCallbackEXT debug = ::debugVulkan ? initDebug( instance ) : VK_NULL_HANDLE;

	VkPhysicalDevice physicalDevice = getPhysicalDevice( instance );
	VkPhysicalDeviceProperties physicalDeviceProperties = getPhysicalDeviceProperties( physicalDevice );
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = getPhysicalDeviceMemoryProperties( physicalDevice );
	uint32_t queueFamily = getQueueFamily( physicalDevice );
	VkDevice device = initDevice(
		physicalDevice, queueFamily,
		layers,
		{VK_KHR_SWAPCHAIN_EXTENSION_NAME}
	);
	VkQueue queue = getQueue( device, queueFamily, 0 );

	PlatformWindow window = initWindow( ::windowWidth, ::windowHeight );
	VkSurfaceKHR surface = initSurface( instance, physicalDevice, queueFamily, window );
	VkSurfaceCapabilitiesKHR surfaceCapabilities = getSurfaceCapabilities( physicalDevice, surface );
	if( surfaceCapabilities.currentExtent.width != ::windowWidth || surfaceCapabilities.currentExtent.height != ::windowHeight ){
		throw "Surface size does not match requested size!";
	}
	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat( physicalDevice, surface );
	VkSwapchainKHR swapchain = initSwapchain( physicalDevice, device, surface, surfaceFormat );
	vector<VkImage> swapchainImages = getSwapchainImages( device, swapchain );
	vector<VkImageView> swapchainImageViews = initSwapchainImageViews( device, swapchainImages, surfaceFormat.format );

	VkRenderPass renderPass = initRenderPass( device, surfaceFormat );

	vector<VkFramebuffer> framebuffers = initFramebuffers( device, renderPass, swapchainImageViews, ::windowWidth, ::windowHeight );

	VkShaderModule vertexShader = initShaderModule( device, ::vertexShaderFilename );
	VkShaderModule fragmentShader = initShaderModule( device, ::fragmentShaderFilename );
	VkPipelineLayout pipelineLayout = initPipelineLayout( device );
	VkPipeline pipeline = initPipeline(
		device,
		physicalDeviceProperties.limits,
		pipelineLayout,
		renderPass,
		vertexShader,
		fragmentShader,
		vertexBufferBinding
	);

	VkBuffer vertexBuffer = initBuffer( device, sizeof( decltype( triangle )::value_type ) * triangle.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );
	VkDeviceMemory vertexBufferMemory = initMemory<ResourceType::Buffer>(
		device,
		physicalDeviceMemoryProperties,
		vertexBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	setVertexData( device, vertexBufferMemory, triangle );

	VkSemaphore imageReadyS = initSemaphore( device );
	VkSemaphore renderDoneS = initSemaphore( device );


	VkCommandPool commandPool = initCommandPool( device, queueFamily );

	vector<VkCommandBuffer> commandBuffers = acquireCommandBuffers(  device, commandPool, static_cast<uint32_t>( swapchainImages.size() )  );
	for( size_t i = 0; i < commandBuffers.size(); ++i ){
		beginCommandBuffer( commandBuffers[i] );
			recordBeginRenderPass( commandBuffers[i], renderPass, framebuffers[i], ::clearColor, ::windowWidth, ::windowHeight );

			recordBindPipeline( commandBuffers[i], pipeline );
			recordBindVertexBuffer( commandBuffers[i], vertexBufferBinding, vertexBuffer );

			recordSetViewport( commandBuffers[i], ::windowWidth, ::windowHeight );
			recordSetScissor( commandBuffers[i], ::windowWidth, ::windowHeight );

			recordDraw( commandBuffers[i], triangle );

			recordEndRenderPass( commandBuffers[i] );
		endCommandBuffer( commandBuffers[i] );
	}

	unsigned frames = 0;
	steady_clock::time_point start = steady_clock::now();

	int ret = EXIT_SUCCESS;
	for( bool quit = false; !quit; ){
		ret = messageLoop( quit ); // process all available events

		// Rendering! Yay!
		//VkResult errorCode = vkQueueWaitIdle( queue ); RESULT_HANDLER( errorCode, "vkQueueWaitIdle" );
		uint32_t nextSwapchainImageIndex = getNextImageIndex( device, swapchain, imageReadyS );
		submitToQueue( queue, commandBuffers[nextSwapchainImageIndex], imageReadyS, renderDoneS );
		present( queue, swapchain, nextSwapchainImageIndex, renderDoneS );
		++frames;
	}

	VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );

	steady_clock::time_point end = steady_clock::now();

	duration<double> time_span = duration_cast<duration<double>>( end - start );

	cout << "Rendered " << frames << " frames in " << time_span.count() << " seconds. Average FPS is " << frames / time_span.count() << endl;

	killCommandPool( device,  commandPool );

	killSemaphore( device, imageReadyS );
	killSemaphore( device, renderDoneS );

	killMemory( device, vertexBufferMemory );
	killBuffer( device, vertexBuffer );

	killPipeline( device, pipeline );
	killPipelineLayout( device, pipelineLayout );
	killShaderModule( device, fragmentShader );
	killShaderModule( device, vertexShader );

	killFramebuffers( device, framebuffers );

	killRenderPass( device, renderPass );

	killSwapchainImageViews( device, swapchainImageViews );
	killSwapchain( device, swapchain );
	killSurface( instance, surface );
	killWindow( window );

	killDevice( device );

	if( ::debugVulkan ) killDebug( instance, debug );
	killInstance( instance );

	return ret;
}
catch( VulkanResultException vkE ){
	cout << "ERROR: Terminated due to an uncaught VkResult exception: "
	     << vkE.file << ":" << vkE.line << ":" << vkE.func << "() " << vkE.source << "() returned " << to_string( vkE.result )
	     << endl;
}
catch( const char* e ){
	cout << "ERROR: Terminated due to an uncaught exception: " << e << endl;
}
catch( string e ){
	cout << "ERROR: Terminated due to an uncaught exception: " << e << endl;
}
catch( std::exception e ){
	cout << "ERROR: Terminated due to an uncaught exception: " << e.what() << endl;
}
catch( ... ){
	cout << "ERROR: Terminated due to an unrecognized uncaught exception." << endl;
}


#if defined(_WIN32) || defined(__CYGWIN__)
// In case of non console subsystem just relay to main()
int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow ){
	UNREFERENCED_PARAMETER( hInstance );
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );
	UNREFERENCED_PARAMETER( nCmdShow );

	return main();
}
#endif

// Platform dependent implementation
//////////////////////////////////
#ifdef VK_USE_PLATFORM_WIN32_KHR
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

	static unsigned uniqueCounter = 0;
	if( uniqueCounter == 1000 ) throw "What the is wrong with you... I mean: too many window class registrations made!";
	std::wstring className =  L"vkwc" + std::to_wstring( uniqueCounter++ );
	WNDCLASSEXW windowClass = {
		sizeof( WNDCLASSEXW ), // cbSize
		/*CS_OWNDC |*/ CS_HREDRAW | CS_VREDRAW, // style - some window behavior
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
		TEXT("Vulkan Test - Hello"),
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

void killSurface( VkInstance instance, VkSurfaceKHR surface ){
	vkDestroySurfaceKHR( instance, surface, nullptr );
}
#endif


// Implementation
//////////////////////////////////
const char* to_string( VkResult r ){
	switch( r ){
		case VK_SUCCESS: return "VK_SUCCESS";
		case VK_NOT_READY: return "VK_NOT_READY";
		case VK_TIMEOUT: return "VK_TIMEOUT";
		case VK_EVENT_SET: return "VK_EVENT_SET";
		case VK_EVENT_RESET: return "VK_EVENT_RESET";
		case VK_INCOMPLETE: return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
		default: return "unrecognized VkResult code";
	}
}

const char* to_string( VkDebugReportObjectTypeEXT o ){
	switch( o ){
		case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT: return "unknown";
		case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT: return "Instance";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT: return "PhysicalDevice";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT: return "Device";
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT: return "Queue";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT: return "Semaphore";
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT: return "CommandBuffer";
		case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT: return "Fence";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT: return "DeviceMemory";
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT: return "Buffer";
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT: return "Image";
		case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT: return "Event";
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT: return "QueryPool";
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT: return "BufferView";
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT: return "ImageView";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT: return "ShaderModule";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT: return "PipelineCache";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT: return "PipelineLayout";
		case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT: return "RenderPass";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT: return "Pipeline";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT: return "DescriptorSetLayout";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT: return "Sampler";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT: return "DescriptorPool";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT: return "DescriptorSet";
		case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT: return "Framebuffer";
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT: return "Command pool";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT: return "SurfaceKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT: return "SwapchainKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT: return "DebugReport";
		default: return "unrecognized";
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkFlags msgFlags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t /*location*/,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* /*pUserData*/
){

	string report = to_string( objType ) + to_string( srcObject ) + ": " + to_string( msgCode ) + ", " + pLayerPrefix + ", " + pMsg;;

	switch( msgFlags ){
		case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
			cout << "Info: " << report << endl;
			break;

		case VK_DEBUG_REPORT_WARNING_BIT_EXT:
			cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout << "WARNING: " << report << endl;
			cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			break;

		case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
			cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout << "PERFORMANCE: " << report << endl;;
			cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			break;

		case VK_DEBUG_REPORT_ERROR_BIT_EXT:
			cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout << "ERROR: " << report << endl;;
			cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			break;

		case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
			report += "Debug: ";
			break;
	}

	// no abort on misbehaving function
	return VK_FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkInstance initInstance( const vector<const char*> layers, const vector<const char*> extensions ){
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = u8"vulkanTest";
	appInfo.pEngineName = u8"myEngine";
	appInfo.apiVersion = 0; // 0 should accept any version

	VkInstanceCreateInfo instanceInfo{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		&appInfo, // has to be non-NULL due to a AMD driver bug
		(uint32_t)layers.size(),
		layers.data(),
		(uint32_t)extensions.size(),
		extensions.data()
	};

	VkDebugReportCallbackCreateInfoEXT debugCreateInfo{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
		nullptr,
		::debugAmount,
		debugCallback,
		nullptr
	};

	if( ::debugVulkan ){
		instanceInfo.pNext = &debugCreateInfo; // valid just for createInstance/destroyInstance execution
	}

	VkInstance instance;
	VkResult errorCode = vkCreateInstance( &instanceInfo, nullptr, &instance ); RESULT_HANDLER( errorCode, "vkCreateInstance" );

	return instance;
}

void killInstance( VkInstance instance ){
	vkDestroyInstance( instance, nullptr );
}

PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT = nullptr;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback
){
	return fpCreateDebugReportCallbackEXT( instance, pCreateInfo, pAllocator, pCallback );
}

PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT = nullptr;
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks* pAllocator
){
	fpDestroyDebugReportCallbackEXT( instance, callback, pAllocator );
}

PFN_vkDebugReportMessageEXT fpDebugReportMessageEXT = nullptr;
VKAPI_ATTR void VKAPI_CALL vkDebugReportMessageEXT(
	VkInstance instance,
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage
){
	fpDebugReportMessageEXT( instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage );
}

void loadVulkanExtensions( VkInstance instance ){
	fpCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
	fpDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
	fpDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr( instance, "vkDebugReportMessageEXT" );
}

VkDebugReportCallbackEXT initDebug( VkInstance instance ){
	VkDebugReportCallbackCreateInfoEXT debugCreateInfo{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
		nullptr,
		::debugAmount,
		debugCallback,
		nullptr
	};

	VkDebugReportCallbackEXT debug;
	VkResult errorCode = vkCreateDebugReportCallbackEXT( instance, &debugCreateInfo, nullptr, &debug ); RESULT_HANDLER( errorCode, "vkCreateDebugReportCallbackEXT" );

	return debug;
}

void killDebug( VkInstance instance, VkDebugReportCallbackEXT debug ){
	vkDestroyDebugReportCallbackEXT( instance, debug, nullptr );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vector<VkPhysicalDevice> getPhysicalDevices( VkInstance instance ){
	vector<VkPhysicalDevice> devices;

	VkResult errorCode;
	do{
		uint32_t deviceCount = 0;
		errorCode = vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr ); RESULT_HANDLER( errorCode, "vkEnumeratePhysicalDevices" );

		devices.resize( deviceCount );
		errorCode = vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data() );
	} while( errorCode == VK_INCOMPLETE );

	RESULT_HANDLER( errorCode, "vkEnumeratePhysicalDevices" );

	return devices;
}

VkPhysicalDevice getPhysicalDevice( VkInstance instance ){
	vector<VkPhysicalDevice> devices = getPhysicalDevices( instance );

	if( devices.empty() ) throw "No VkPhysicalDevice found!";
	else if( devices.size() > 1 ){
		cout << "WARNING: More than one VkPhysicalDevice found - just picking the first one.\n";
	}

	return devices[0];
}

VkPhysicalDeviceProperties getPhysicalDeviceProperties( VkPhysicalDevice physicalDevice ){
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties( physicalDevice, &properties );
	return properties;
}

VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties( VkPhysicalDevice physicalDevice ){
	VkPhysicalDeviceMemoryProperties memoryInfo;
	vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memoryInfo );
	return memoryInfo;
}

vector<VkQueueFamilyProperties> getQueueFamilyProperties( VkPhysicalDevice device ){
	vector<VkQueueFamilyProperties> queueFamilies;

	uint32_t queueFamiliesCount;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, nullptr );

	queueFamilies.resize( queueFamiliesCount );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, queueFamilies.data() );

	return queueFamilies;
}

uint32_t getQueueFamily( VkPhysicalDevice physDevice ){
	auto qfps = getQueueFamilyProperties( physDevice );
	uint32_t qfi = 0;
	while(  !( qfps[qfi].queueFlags & VK_QUEUE_GRAPHICS_BIT )  ) ++qfi;

	if(  !presentationSupport( physDevice, qfi )  ) throw "Selected queue family of physical device can't present to a surface!";

	return qfi;
}

VkDevice initDevice( VkPhysicalDevice physDevice, uint32_t queueFamilyIndex, vector<const char*> layers, vector<const char*> extensions ){
	VkPhysicalDeviceFeatures ft;
	vkGetPhysicalDeviceFeatures( physDevice, &ft ); // all features
	return initDevice( physDevice, ft, queueFamilyIndex, layers, extensions );
}

VkDevice initDevice(
	VkPhysicalDevice physDevice,
	const VkPhysicalDeviceFeatures& features,
	uint32_t queueFamilyIndex,
	vector<const char*> layers,
	vector<const char*> extensions
){

	const float priority[] = {1.0f};
	VkDeviceQueueCreateInfo qci{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr,
		0,
		queueFamilyIndex,
		1,
		priority
	};

	vector<VkDeviceQueueCreateInfo> queues;
	queues.push_back( qci );

	VkDeviceCreateInfo deviceInfo{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		(uint32_t)queues.size(),
		queues.data(),
		(uint32_t)layers.size(),
		layers.data(),
		(uint32_t)extensions.size(),
		extensions.data(),
		&features
	};


	VkDevice device;
	VkResult errorCode = vkCreateDevice( physDevice, &deviceInfo, nullptr, &device ); RESULT_HANDLER( errorCode, "vkCreateDevice" );

	return device;
}

void killDevice( VkDevice device ){
	vkDestroyDevice( device, nullptr );
}

VkQueue getQueue( VkDevice device, uint32_t queueFamily, uint32_t queueIndex ){
	VkQueue queue;
	vkGetDeviceQueue( device, queueFamily, queueIndex, &queue );

	return queue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< ResourceType resourceType, class T >
VkMemoryRequirements getMemoryRequirements( VkDevice device, T resource );

template<>
VkMemoryRequirements getMemoryRequirements< ResourceType::Buffer >( VkDevice device, VkBuffer buffer ){
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements( device, buffer, &memoryRequirements );

	return memoryRequirements;
}

template<>
VkMemoryRequirements getMemoryRequirements< ResourceType::Image >( VkDevice device, VkImage image ){
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements( device, image, &memoryRequirements );

	return memoryRequirements;
}

template< ResourceType resourceType, class T >
void bindMemory( VkDevice device, T buffer, VkDeviceMemory memory, VkDeviceSize offset );

template<>
void bindMemory< ResourceType::Buffer >( VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset ){
	VkResult errorCode = vkBindBufferMemory( device, buffer, memory, offset ); RESULT_HANDLER( errorCode, "vkBindBufferMemory" );
}

template<>
void bindMemory< ResourceType::Image >( VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize offset ){
	VkResult errorCode = vkBindImageMemory( device, image, memory, offset ); RESULT_HANDLER( errorCode, "vkBindImageMemory" );
}

template< ResourceType resourceType, class T >
VkDeviceMemory initMemory(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	T resource,
	VkMemoryPropertyFlags requiredFlags,
	VkMemoryPropertyFlags desiredFlags
){
	VkMemoryRequirements memoryRequirements = getMemoryRequirements<resourceType>( device, resource );

	uint32_t memoryType = 0;
	bool found = false;

	for( uint32_t i = 0; i < 32; ++i ){
		if( memoryRequirements.memoryTypeBits & 0x1 ){
			if( (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags ){
				if( !found ){
					memoryType = i;
					found = true;
				}
				else if( /*found &&*/ (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & desiredFlags) == desiredFlags ){
					memoryType = i;
				}
			}
		}

		memoryRequirements.memoryTypeBits >>= 1;
	}

	if( !found ) throw "Can't find compatible mappable memory for the resource";

	VkMemoryAllocateInfo memoryInfo{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr, // pNext
		memoryRequirements.size,
		memoryType
	};

	VkDeviceMemory memory;
	VkResult errorCode = vkAllocateMemory( device, &memoryInfo, nullptr, &memory ); RESULT_HANDLER( errorCode, "vkAllocateMemory" );

	bindMemory<resourceType>( device, resource, memory, 0 /*offset*/ );

	return memory;
}

void setMemoryData( VkDevice device, VkDeviceMemory memory, void* begin, size_t size ){
	void* data;
	VkResult errorCode = vkMapMemory( device, memory, 0 /*offset*/, VK_WHOLE_SIZE, 0 /*flags - reserved*/, &data ); RESULT_HANDLER( errorCode, "vkMapMemory" );
	memcpy( data, begin, size );
	vkUnmapMemory( device, memory );
}

void killMemory( VkDevice device, VkDeviceMemory memory ){
	vkFreeMemory( device, memory, nullptr );
}


VkBuffer initBuffer( VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage ){
	VkBufferCreateInfo bufferInfo{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr, // pNext
		0, // flags
		size,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, // queue family count -- ignored for EXCLUSIVE
		nullptr // queue families -- ignored for EXCLUSIVE
	};

	VkBuffer buffer;
	VkResult errorCode = vkCreateBuffer( device, &bufferInfo, nullptr, &buffer ); RESULT_HANDLER( errorCode, "vkCreateBuffer" );
	return buffer;
}

void killBuffer( VkDevice device, VkBuffer buffer ){
	vkDestroyBuffer( device, buffer, nullptr );
}

VkImage initImage( VkDevice device, VkFormat format, uint32_t width, uint32_t height, VkSampleCountFlagBits samples, VkImageUsageFlags usage ){
	VkExtent3D size{
		width,
		height,
		1 // depth
	};

	VkImageCreateInfo ici{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr, // pNext
		0, // flags
		VK_IMAGE_TYPE_2D,
		format,
		size,
		1, // mipLevels
		1, // arrayLayers
		samples,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, // queueFamilyIndexCount -- ignored for EXCLUSIVE
		nullptr, // pQueueFamilyIndices -- ignored for EXCLUSIVE
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkImage image;
	VkResult errorCode = vkCreateImage( device, &ici, nullptr, &image ); RESULT_HANDLER( errorCode, "vkCreateImage" );

	return image;
}

void killImage( VkDevice device, VkImage image ){
	vkDestroyImage( device, image, nullptr );
}

VkImageView initImageView( VkDevice device, VkImage image, VkFormat format ){
	VkImageViewCreateInfo iciv{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr, // pNext
		0, // flags
		image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			/* base mip-level */ 0,
			/* level count */ VK_REMAINING_MIP_LEVELS,
			/* base array layer */ 0,
			/* array layer count */ VK_REMAINING_ARRAY_LAYERS
		}
	};

	VkImageView imageView;
	VkResult errorCode = vkCreateImageView( device, &iciv, nullptr, &imageView ); RESULT_HANDLER( errorCode, "vkCreateImageView" );

	return imageView;
}

void killImageView( VkDevice device, VkImageView imageView ){
	vkDestroyImageView( device, imageView, nullptr );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vector<VkSurfaceFormatKHR> getSurfaceFormats( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	vector<VkSurfaceFormatKHR> formats;

	VkResult errorCode;
	do{
		uint32_t formatCount = 0;
		errorCode = vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, nullptr ); RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfaceFormatsKHR" );

		formats.resize( formatCount );
		errorCode = vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, formats.data() );
	} while( errorCode == VK_INCOMPLETE );

	RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfaceFormatsKHR" );

	return formats;
}

VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	vector<VkSurfaceFormatKHR> formats = getSurfaceFormats( physicalDevice, surface );

	for( auto f : formats ){
		if( f.format == /*VK_FORMAT_B8G8R8A8_SRGB*/ VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR ){
			return f;
		}
	}

	throw "No suitable Surface format found!";
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	VkSurfaceCapabilitiesKHR capabilities;
	VkResult errorCode = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &capabilities ); RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR" );

	return capabilities;
}

vector<VkPresentModeKHR> getSurfacePresentModes( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	vector<VkPresentModeKHR> modes;

	VkResult errorCode;
	do{
		uint32_t modeCount = 0;
		errorCode = vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &modeCount, nullptr ); RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfacePresentModesKHR" );

		modes.resize( modeCount );
		errorCode = vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &modeCount, modes.data() );
	} while( errorCode == VK_INCOMPLETE );

	RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfacePresentModesKHR" );

	return modes;
}

VkPresentModeKHR getSurfacePresentMode( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	vector<VkPresentModeKHR> modes = getSurfacePresentModes( physicalDevice, surface );

	for( auto m : modes ){
		if( m == ::presentMode ) return m;
	}

	throw "Your prefered present mode is not supported! Adjust your config or try FIFO (always supported).";
}

VkSwapchainKHR initSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat ){
	VkSurfaceCapabilitiesKHR capabilities = getSurfaceCapabilities( physicalDevice, surface );

	if(  !( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT )  ){
		throw "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT not supported!";
	}
	// TODO: perhaps not really necessary
	if(  !( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR )  ){
		throw "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR not supported!";
	}

	VkSwapchainCreateInfoKHR swapchainInfo{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		surface,
		capabilities.minImageCount, // minImageCount
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		capabilities.currentExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // VkImage usage flags
		VK_SHARING_MODE_EXCLUSIVE,
		0, // sharing queue families count
		nullptr, // sharing queue families
		capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		getSurfacePresentMode( physicalDevice, surface ),
		VK_TRUE, // clipped
		VK_NULL_HANDLE
	};

	VkSwapchainKHR swapchain;
	VkResult errorCode = vkCreateSwapchainKHR( device, &swapchainInfo, nullptr, &swapchain ); RESULT_HANDLER( errorCode, "vkCreateSwapchainKHR" );

	return swapchain;
}

void killSwapchain( VkDevice device, VkSwapchainKHR swapchain ){
	vkDestroySwapchainKHR( device, swapchain, nullptr );
}

vector<VkImage> getSwapchainImages( VkDevice device, VkSwapchainKHR swapchain ){
	vector<VkImage> images;

	VkResult errorCode;
	do{
		uint32_t imageCount = 0;
		errorCode = vkGetSwapchainImagesKHR( device, swapchain, &imageCount, nullptr ); RESULT_HANDLER( errorCode, "vkGetSwapchainImagesKHR" );

		images.resize( imageCount );
		errorCode = vkGetSwapchainImagesKHR(  device, swapchain, &imageCount, images.data() );
	} while( errorCode == VK_INCOMPLETE );

	RESULT_HANDLER( errorCode, "vkGetSwapchainImagesKHR" );

	return images;
}

uint32_t getNextImageIndex( VkDevice device, VkSwapchainKHR swapchain, VkSemaphore imageReadyS ){
	uint32_t nextImageIndex;
	VkResult errorCode = vkAcquireNextImageKHR(
		device,
		swapchain,
		UINT64_MAX /* no timeout */,
		imageReadyS,
		VK_NULL_HANDLE,
		&nextImageIndex
	); RESULT_HANDLER( errorCode, "vkAcquireNextImageKHR" );

	return nextImageIndex;
}

vector<VkImageView> initSwapchainImageViews( VkDevice device, vector<VkImage> images, VkFormat format ){
	vector<VkImageView> imageViews;

	for( auto image : images ){
		VkImageView imageView = initImageView( device, image, format );

		imageViews.push_back( imageView );
	}

	return imageViews;
}

void killSwapchainImageViews( VkDevice device, vector<VkImageView> imageViews ){
	for( auto imageView : imageViews ) vkDestroyImageView( device, imageView, nullptr );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkRenderPass initRenderPass( VkDevice device, VkSurfaceFormatKHR surfaceFormat ){
	VkAttachmentDescription colorAtachment{
		0, // flags
		surfaceFormat.format,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR, // color + depth
		VK_ATTACHMENT_STORE_OP_STORE, // color + depth
		VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil
		VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference colorReference{
		0, // attachment
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass{
		0, // flags - reserved for future use
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0, // input attachment count
		nullptr, // input attachments
		1, // color attachment count
		&colorReference, // color attachments
		nullptr, // resolve attachments
		nullptr, // depth stencil attachment
		0, // preserve attachment count
		nullptr // preserve attachments
	};

	VkSubpassDependency srcDependency{
		VK_SUBPASS_EXTERNAL, // srcSubpass
		0, // dstSubpass
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
		VK_ACCESS_MEMORY_READ_BIT, // srcAccessMask
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // dstAccessMask
		VK_DEPENDENCY_BY_REGION_BIT, // dependencyFlags
	};

	VkSubpassDependency dstDependency{
		0, // srcSubpass
		VK_SUBPASS_EXTERNAL, // dstSubpass
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // srcAccessMask
		VK_ACCESS_MEMORY_READ_BIT, // dstAccessMask
		VK_DEPENDENCY_BY_REGION_BIT, // dependencyFlags
	};

	VkSubpassDependency dependencies[] = {srcDependency, dstDependency};

	VkRenderPassCreateInfo renderPassInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		1, // attachment count
		&colorAtachment, // attachments
		1, // subpass count
		&subpass, // subpasses
		2, // dependency count
		dependencies // dependencies
	};

	VkRenderPass renderPass;
	VkResult errorCode = vkCreateRenderPass( device, &renderPassInfo, nullptr, &renderPass ); RESULT_HANDLER( errorCode, "vkCreateRenderPass" );

	return renderPass;
}

void killRenderPass( VkDevice device, VkRenderPass renderPass ){
	vkDestroyRenderPass( device, renderPass, nullptr );
}

vector<VkFramebuffer> initFramebuffers(
	VkDevice device,
	VkRenderPass renderPass,
	vector<VkImageView> imageViews,
	uint32_t width, uint32_t height
){
	vector<VkFramebuffer> framebuffers;

	for( auto imageView : imageViews ){
		VkFramebufferCreateInfo framebufferInfo{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr, // pNext
			0, // flags - reserved for future use
			renderPass,
			1, // ImageView count
			&imageView,
			width, // width
			height, // height
			1 // layers
		};

		VkFramebuffer framebuffer;
		VkResult errorCode = vkCreateFramebuffer( device, &framebufferInfo, nullptr, &framebuffer ); RESULT_HANDLER( errorCode, "vkCreateFramebuffer" );
		framebuffers.push_back( framebuffer );
	}

	return framebuffers;
}

void killFramebuffers( VkDevice device, vector<VkFramebuffer> framebuffers ){
	for( auto framebuffer : framebuffers ) vkDestroyFramebuffer( device, framebuffer, nullptr );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderModule initShaderModule( VkDevice device, string filename ){
	//using Fs = basic_ifstream<uint32_t>;
	using Fs = ifstream;
	Fs ifs( filename, Fs::in | Fs::binary );
	if( !ifs.is_open() ) throw string( "SPIR-V shader file failed to open: " ) + strerror( errno );
	vector<char> shaderCode( (istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>() /* EOS */ ); // Most Vexing Parse

	if( shaderCode.empty() || shaderCode.size() % 4 != 0 /* per spec; % sizeof(uint32_t) presumably */ ){
		throw "SPIR-V shader file is invalid or read failed!";
	}

	VkShaderModuleCreateInfo shaderModuleInfo{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		shaderCode.size(),
		reinterpret_cast<const uint32_t*>( shaderCode.data() )
	};


	VkShaderModule shaderModule;
	VkResult errorCode = vkCreateShaderModule( device, &shaderModuleInfo, nullptr, &shaderModule ); RESULT_HANDLER( errorCode, "vkCreateShaderModule" );

	return shaderModule;
}

void killShaderModule( VkDevice device, VkShaderModule shaderModule ){
	vkDestroyShaderModule( device, shaderModule, nullptr );
}

VkPipelineLayout initPipelineLayout( VkDevice device ){
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		0, // descriptorSetLayout count
		nullptr,
		0, // push constant range count
		nullptr // push constant ranges
	};

	VkPipelineLayout pipelineLayout;
	VkResult errorCode = vkCreatePipelineLayout( device, &pipelineLayoutInfo, nullptr, &pipelineLayout ); RESULT_HANDLER( errorCode, "vkCreatePipelineLayout" );

	return pipelineLayout;
}

void killPipelineLayout( VkDevice device, VkPipelineLayout pipelineLayout ){
	vkDestroyPipelineLayout( device, pipelineLayout, nullptr );
}

VkPipeline initPipeline(
	VkDevice device,
	VkPhysicalDeviceLimits limits,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass,
	VkShaderModule vertexShader,
	VkShaderModule fragmentShader,
	const uint32_t vertexBufferBinding
){
	const VkPipelineShaderStageCreateInfo vertexShaderStage{
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		VK_SHADER_STAGE_VERTEX_BIT,
		vertexShader,
		u8"main",
		nullptr // SpecializationInfo - constants pushed to shader on pipeline creation time
	};

	const VkPipelineShaderStageCreateInfo fragmentShaderStage{
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		VK_SHADER_STAGE_FRAGMENT_BIT,
		fragmentShader,
		u8"main",
		nullptr // SpecializationInfo - constants pushed to shader on pipeline creation time
	};

	vector<VkPipelineShaderStageCreateInfo> shaderStageStates = { vertexShaderStage, fragmentShaderStage };

	const uint32_t vertexBufferStride = sizeof( Vertex );
	if( vertexBufferBinding > limits.maxVertexInputBindings ){
		throw string("Implementation does not allow enough input bindings. Needed: ")
		    + to_string( vertexBufferBinding ) + string(", max: ")
		    + to_string( limits.maxVertexInputBindings );
	}
	if( vertexBufferStride > limits.maxVertexInputBindingStride ){
		throw string("Implementation does not allow big enough vertex buffer stride: ")
		    + to_string( vertexBufferStride ) 
		    + string(", max: ")
		    + to_string( limits.maxVertexInputBindingStride );
	}

	VkVertexInputBindingDescription vertexInputBindingDescription{
		vertexBufferBinding,
		sizeof( Vertex ), // stride in bytes
		VK_VERTEX_INPUT_RATE_VERTEX
	};

	vector<VkVertexInputBindingDescription> inputBindingDescriptions = { vertexInputBindingDescription };
	if( inputBindingDescriptions.size() > limits.maxVertexInputBindings ){
		throw "Implementation does not allow enough input bindings.";
	}

	const uint32_t positionLocation = 0;
	const uint32_t colorLocation = 1;

	if( colorLocation >= limits.maxVertexInputAttributes ){
		throw "Implementation does not allow enough input attributes.";
	}
	if( offsetof( Vertex, color ) > limits.maxVertexInputAttributeOffset ){
		throw "Implementation does not allow sufficient attribute offset.";
	}

	VkVertexInputAttributeDescription positionInputAttributeDescription{
		positionLocation,
		vertexBufferBinding,
		VK_FORMAT_R32G32_SFLOAT,
		offsetof( Vertex, position ) // offset in bytes
	};

	VkVertexInputAttributeDescription colorInputAttributeDescription{
		colorLocation,
		vertexBufferBinding,
		VK_FORMAT_R32G32B32_SFLOAT,
		offsetof( Vertex, color ) // offset in bytes
	};

	vector<VkVertexInputAttributeDescription> inputAttributeDescriptions = {
		positionInputAttributeDescription,
		colorInputAttributeDescription
	};

	VkPipelineVertexInputStateCreateInfo vertexInputState{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		static_cast<uint32_t>( inputBindingDescriptions.size() ),
		inputBindingDescriptions.data(),
		static_cast<uint32_t>( inputAttributeDescriptions.size() ),
		inputAttributeDescriptions.data()
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE // primitive restart
	};

	VkPipelineViewportStateCreateInfo viewportState{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		1, // Viewport count
		nullptr, // vieports - ignored for dynamic viewport
		1, // scisor count,
		nullptr // scisors - ignored for dynamic scisors
	};

	VkPipelineRasterizationStateCreateInfo rasterizationState{
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		VK_FALSE, // depth clamp
		VK_FALSE, // rasterizer discard
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, // depth bias
		0.0f, // bias constant factor
		0.0f, // bias clamp
		0.0f, // bias slope factor
		0.0f // line width
	};

	VkPipelineMultisampleStateCreateInfo multisampleState{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, // no sample shading
		0.0f, // min sample shading - ignored if disabled
		nullptr, // sample mask
		VK_FALSE, // alphaToCoverage
		VK_FALSE // alphaToOne
	};

	VkPipelineColorBlendAttachmentState blendAttachmentState{
		VK_FALSE, // blending enabled?
		VK_BLEND_FACTOR_ZERO, // src blend factor -ignored?
		VK_BLEND_FACTOR_ZERO, // dst blend factor
		VK_BLEND_OP_ADD, // blend op
		VK_BLEND_FACTOR_ZERO, // src alpha blend factor
		VK_BLEND_FACTOR_ZERO, // dst alpha blend factor
		VK_BLEND_OP_ADD, // alpha blend op
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT // color write mask
	};

	VkPipelineColorBlendStateCreateInfo colorBlendState{
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		VK_FALSE, // logic ops
		VK_LOGIC_OP_COPY,
		1, // attachment count - must be same as color attachment count in renderpass subpass!
		&blendAttachmentState,
		{0.0f, 0.0f, 0.0f, 0.0f} // blend constants
	};

	vector<VkDynamicState> dynamicObjects = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		static_cast<uint32_t>( dynamicObjects.size() ),
		dynamicObjects.data()
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - e.g. disable optimization
		static_cast<uint32_t>( shaderStageStates.size() ), // shader stages count - vertex and fragment
		shaderStageStates.data(),
		&vertexInputState,
		&inputAssemblyState,
		nullptr, // tesselation
		&viewportState,
		&rasterizationState,
		&multisampleState,
		nullptr, // depth stencil
		&colorBlendState,
		&dynamicState, // dynamic state
		pipelineLayout,
		renderPass,
		0, // subpass index in renderpass
		VK_NULL_HANDLE, // base pipeline
		-1 // base pipeline index
	};

	VkPipeline pipeline;
	VkResult errorCode = vkCreateGraphicsPipelines(
		device,
		VK_NULL_HANDLE /* pipeline cache */,
		1 /* info count */,
		&pipelineInfo,
		nullptr,
		&pipeline
	); RESULT_HANDLER( errorCode, "vkCreateGraphicsPipelines" );
	return pipeline;
}

void killPipeline( VkDevice device, VkPipeline pipeline ){
	vkDestroyPipeline( device, pipeline, nullptr );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setVertexData( VkDevice device, VkDeviceMemory memory, vector<Vertex> vertices ){
	setMemoryData(  device, memory, vertices.data(), sizeof( decltype(vertices)::value_type ) * vertices.size()  );
}

VkSemaphore initSemaphore( VkDevice device ){
	VkSemaphoreCreateInfo semaphoreInfo{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
	};

	VkSemaphore semaphore;
	VkResult errorCode = vkCreateSemaphore( device, &semaphoreInfo, nullptr, &semaphore ); RESULT_HANDLER( errorCode, "vkCreateSemaphore" );
	return semaphore;
}

void killSemaphore( VkDevice device, VkSemaphore semaphore ){
	vkDestroySemaphore( device, semaphore, nullptr );
}

VkCommandPool initCommandPool( VkDevice device, const uint32_t queueFamily ){
	VkCommandPoolCreateInfo commandPoolInfo{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr, // pNext
		0, // flags
		queueFamily
	};

	VkCommandPool commandPool;
	VkResult errorCode = vkCreateCommandPool( device, &commandPoolInfo, nullptr, &commandPool ); RESULT_HANDLER( errorCode, "vkCreateCommandPool" );
	return commandPool;
}

void killCommandPool( VkDevice device, VkCommandPool commandPool ){
	vkDestroyCommandPool( device, commandPool, nullptr );
}

vector<VkCommandBuffer> acquireCommandBuffers( VkDevice device, VkCommandPool commandPool, uint32_t count ){
	VkCommandBufferAllocateInfo commandBufferInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr, // pNext
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		count // count
	};

	vector<VkCommandBuffer> commandBuffers( count );
	VkResult errorCode = vkAllocateCommandBuffers( device, &commandBufferInfo, commandBuffers.data() ); RESULT_HANDLER( errorCode, "vkAllocateCommandBuffers" );
	return commandBuffers;
}

void beginCommandBuffer( VkCommandBuffer commandBuffer ){
	VkCommandBufferBeginInfo commandBufferInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr, // pNext
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // flags
		nullptr // inheritance
	};

	VkResult errorCode = vkBeginCommandBuffer( commandBuffer, &commandBufferInfo ); RESULT_HANDLER( errorCode, "vkBeginCommandBuffer" );
}

void endCommandBuffer( VkCommandBuffer commandBuffer ){
	VkResult errorCode = vkEndCommandBuffer( commandBuffer ); RESULT_HANDLER( errorCode, "vkEndCommandBuffer" );
}


void recordBeginRenderPass(
	VkCommandBuffer commandBuffer,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	VkClearValue clearValue,
	uint32_t width, uint32_t height
){
	VkRenderPassBeginInfo renderPassInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		nullptr, // pNext
		renderPass,
		framebuffer,
		{{0,0}, {width,height}}, //render area - offset plus extent
		1, // clear value count
		&clearValue
	};

	vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void recordEndRenderPass( VkCommandBuffer commandBuffer ){
	vkCmdEndRenderPass( commandBuffer );
}

void recordBindPipeline( VkCommandBuffer commandBuffer, VkPipeline pipeline ){
	vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
}

void recordBindVertexBuffer( VkCommandBuffer commandBuffer, const uint32_t vertexBufferBinding, VkBuffer vertexBuffer ){
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers( commandBuffer, vertexBufferBinding, 1 /*binding count*/, &vertexBuffer, offsets );
}

void recordSetViewport( VkCommandBuffer commandBuffer, uint32_t width, uint32_t height ){
	VkViewport viewport{
		0.0f, // x
		0.0f, // y
		(float)width,
		(float)height,
		0.0f, // min depth
		1.0f // max depth
	};

	vkCmdSetViewport( commandBuffer, 0 /*first*/, 1 /*count*/, &viewport );
}

void recordSetScissor( VkCommandBuffer commandBuffer, uint32_t width, uint32_t height ){
	VkRect2D scissor{
		{0, 0}, // offset
		{width, height}
	};

	vkCmdSetScissor( commandBuffer, 0 /*first*/, 1 /*count*/, &scissor );
}

void recordDraw( VkCommandBuffer commandBuffer, const vector<Vertex> vertices ){
	vkCmdDraw( commandBuffer, static_cast<uint32_t>( vertices.size() ), 1 /*instance count*/, 0 /*first vertex*/, 0 /*first instance*/ );
}

void submitToQueue( VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore imageReadyS, VkSemaphore renderDoneS ){
	VkPipelineStageFlags psw = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr, // pNext
		1, // wait semaphore count
		&imageReadyS, // semaphores
		&psw, // pipeline stages to wait for semaphore
		1,
		&commandBuffer,
		1, // signal semaphore count
		&renderDoneS // signal semaphores
	};

	VkResult errorCode = vkQueueSubmit( queue, 1 /*submit count*/, &submit, VK_NULL_HANDLE /*fence*/); RESULT_HANDLER( errorCode, "vkQueueSubmit" );
}

void present( VkQueue queue, VkSwapchainKHR swapchain, uint32_t swapchainImageIndex, VkSemaphore renderDoneS ){
	//VkResult errorCodeSwapchain;

	VkPresentInfoKHR presentInfo{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr, // pNext
		1, // wait semaphore count
		&renderDoneS, // wait semaphores
		1, // swapchain count
		&swapchain,
		&swapchainImageIndex,
		nullptr//&errorCodeSwapchain
	};

	VkResult errorCode = vkQueuePresentKHR( queue, &presentInfo ); RESULT_HANDLER( errorCode, "vkQueuePresentKHR" );
	//RESULT_HANDLER( errorCodeSwapchain, "vkQueuePresentKHR" );
}
