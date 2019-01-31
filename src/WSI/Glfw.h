// GLFW platform dependent WSI handling and event loop

#ifndef COMMON_GLFW_WSI_H
#define COMMON_GLFW_WSI_H

#include <functional>
#include <string>
#include <queue>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE // Actually means include no OpenGL header
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "CompilerMessages.h"
#include "ErrorHandling.h"


TODO( "Easier to use, but might prevent platform co-existence. Could be namespaced. Make all of this a class?" )
struct PlatformWindow{ GLFWwindow* window; };

std::string getPlatformSurfaceExtensionName();

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

struct GlfwError{
	int error;
	std::string description;
};

std::queue<GlfwError> errors;

void glfwErrorCallback( int error, const char* description ){
	errors.push( {error, description} ); // postpone errors because I don't want to throw into C API
}


// just make the global glfw instialization\destruction static
class GlfwSingleton{
	static const GlfwSingleton glfwInstance;

	void destroyGlfw() noexcept{
		glfwTerminate();
		glfwSetErrorCallback( nullptr );
	}

	~GlfwSingleton(){
		this->destroyGlfw();
	}

	GlfwSingleton(){
		glfwSetErrorCallback( glfwErrorCallback );

		const auto success = glfwInit();
		if( !success ){
			glfwSetErrorCallback( nullptr );
			throw "Trouble initializing GLFW!";
		}

		if( !glfwVulkanSupported() ){
			this->destroyGlfw();
			throw "GLFW has trouble acquiring Vulkan support!";
		}
	}
};
const GlfwSingleton GlfwSingleton::glfwInstance;

bool hasSwapchain = false;

void showWindow( PlatformWindow window ){
	glfwShowWindow( window.window );

	// on linux there is no automatic initial size event -- need to call explicitly
	if( !hasSwapchain ) hasSwapchain = sizeEventHandler(); 
}

std::string getPlatformSurfaceExtensionName(){
	using std::string;

	uint32_t count;
	const char** extensions = glfwGetRequiredInstanceExtensions( &count ); // GLFW owns **extensions!!!

	if( !count || !extensions ) throw "GLFW failed to return required Vulkan extensions!";

	if( count != 2 ) throw "GLFW returned unexpected number of required Vulkan extensions!";

	if(  extensions[0] != string( VK_KHR_SURFACE_EXTENSION_NAME )  ) throw VK_KHR_SURFACE_EXTENSION_NAME " is not a 1st required GLFW extension!";

	string surfaceExtension = string( extensions[1] );

	return string( extensions[1] );
}

GLFWmonitor* getCurrentMonitor( GLFWwindow* window ){
	using std::max;
	using std::min;

	int bestoverlap = 0;
	GLFWmonitor* bestmonitor = NULL;

	int wx, wy;
	glfwGetWindowPos( window, &wx, &wy );
	int ww, wh;
	glfwGetWindowSize( window, &ww, &wh );

	int nmonitors;
	auto monitors = glfwGetMonitors( &nmonitors );

	for( int i = 0; i < nmonitors; ++i ){
		auto mode = glfwGetVideoMode( monitors[i] );

		int mw = mode->width;
		int mh = mode->height;

		int mx, my;
		glfwGetMonitorPos( monitors[i], &mx, &my );

		int overlap = max(0, min(wx + ww, mx + mw) - max(wx, mx)) * max(0, min(wy + wh, my + mh) - max(wy, my));

		if( bestoverlap < overlap ){
			bestoverlap = overlap;
			bestmonitor = monitors[i];
		}
	}

	return bestmonitor;
}

void windowSizeCallback( GLFWwindow*, int, int ) noexcept{
	hasSwapchain = sizeEventHandler();
}

void windowRefreshCallback( GLFWwindow* ) noexcept{
	//logger << "refresh" << std::endl;
	if( hasSwapchain ) paintEventHandler();
}

void toggleFullscreen( GLFWwindow* window ){
	bool fullscreen = glfwGetWindowMonitor( window ) != NULL;

	TODO( "All of this needs to be made a class... death to the static!" )
	static int wx = 100;
	static int wy = 100;
	static int ww = 800;
	static int wh = 800;

	if( !fullscreen ){
		glfwGetWindowPos( window, &wx, &wy );
		glfwGetWindowSize( window, &ww, &wh );

		GLFWmonitor* monitor = getCurrentMonitor( window );
		const GLFWvidmode* vmode = glfwGetVideoMode( monitor );
		glfwSetWindowMonitor( window, monitor, 0, 0, vmode->width, vmode->height, vmode->refreshRate );
	}
	else{
		glfwSetWindowMonitor( window, NULL, wx, wy, ww, wh, GLFW_DONT_CARE );
	}
}

void keyCallback( GLFWwindow* window, int key, int /*scancode*/, int action, int mods ) noexcept{
	if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) glfwSetWindowShouldClose( window, GLFW_TRUE );

	if( key == GLFW_KEY_ENTER && action == GLFW_PRESS && mods == GLFW_MOD_ALT ) toggleFullscreen( window );
}

int messageLoop( PlatformWindow window ){
	using std::to_string;

	while(  errors.empty() && !glfwWindowShouldClose( window.window )  ){
		if( hasSwapchain ) glfwPollEvents(); // do not block so I can paint
		else glfwWaitEvents(); // allows blocking if no events

		if( hasSwapchain ) paintEventHandler(); // repaint always even without OS repaint event
	}

	if( !errors.empty() ) throw to_string( errors.size() ) + " GLFW error(s) on backlog; 1st error: " + to_string( errors.front().error ) + ": " + errors.front().description;

	return EXIT_SUCCESS;
}


bool platformPresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex, PlatformWindow ){
	return glfwGetPhysicalDevicePresentationSupport( instance, device, queueFamilyIndex ) == GLFW_TRUE;
}

PlatformWindow initWindow( const std::string& name, const uint32_t canvasWidth, const uint32_t canvasHeight ){
	std::string title = name + " -- GLFW";

	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE );
	GLFWwindow* window = glfwCreateWindow( static_cast<int>(canvasWidth), static_cast<int>(canvasHeight), title.c_str(), NULL, NULL );

	if( !window ) throw "Trouble creating GLFW window!";

	glfwSetInputMode( window, GLFW_STICKY_KEYS, GLFW_TRUE );

	glfwSetFramebufferSizeCallback( window, windowSizeCallback );
	glfwSetWindowRefreshCallback( window, windowRefreshCallback );
	glfwSetKeyCallback( window, keyCallback );

	return { window };
}

void killWindow( PlatformWindow window ){
	glfwDestroyWindow( window.window );
}

VkSurfaceKHR initSurface( const VkInstance instance, const PlatformWindow window ){
	VkSurfaceKHR surface;
	const VkResult errorCode = glfwCreateWindowSurface( instance, window.window, nullptr, &surface ); RESULT_HANDLER( errorCode, "glfwCreateWindowSurface" );

	return surface;
}


#endif //COMMON_GLFW_WSI_H
