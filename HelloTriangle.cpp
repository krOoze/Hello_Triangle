// Vulkan hello world triangle rendering demo


// Global header settings
//////////////////////////////////////////////////////////////////////////////////

#include "VulkanEnvironment.h" // first include must be before vulkan.h and platform header


// Includes
//////////////////////////////////////////////////////////////////////////////////

#include <vector>
using std::vector;

#include<cstring>
#include <string>
using std::string;

#include <exception>
#include <stdexcept>
using std::exception;
using std::runtime_error;

#include <fstream>
#include <iterator>

#include <cmath>
#include <algorithm>
#include <functional>

#include <cassert>

#include <vulkan/vulkan.h> // + assume core+WSI is loaded
#if VK_HEADER_VERSION < 46
	#error Update your SDK! This app is written against Vulkan header version 46
#endif

#include "to_string.h"
#include "ErrorHandling.h"
#include "Vertex.h"
#include "EnumerateScheme.h"
#include "ExtensionLoader.h"

#if defined(USE_PLATFORM_GLFW)
	#include "glfwPlatform.h"
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	#include "win32Platform.h"
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	#include "xlibPlatform.h"
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	#include "xcbPlatform.h"
#else
	#error "Unsupported Vulkan WSI platform."
#endif


// Config
//////////////////////////////////////////////////////////////////////////////////

// layers and debug
#if VULKAN_VALIDATION
	constexpr VkDebugReportFlagsEXT debugAmount =
		0
		//| VK_DEBUG_REPORT_INFORMATION_BIT_EXT
		| VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_ERROR_BIT_EXT
		//| VK_DEBUG_REPORT_DEBUG_BIT_EXT
	;
#endif

constexpr bool fpsCounter = true;

// window and swapchain
constexpr int initialWindowWidth = 800;
constexpr int initialWindowHeight = 800;

//constexpr VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // better not be used often because of coil whine
constexpr VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

// pipeline settings
constexpr VkClearValue clearColor = {  { {0.1f, 0.1f, 0.1f, 1.0f} }  };

constexpr char vertexShaderFilename[] = "triangle.vert.spv";
constexpr char fragmentShaderFilename[] = "triangle.frag.spv";


// needed stuff for main() -- forward declarations
//////////////////////////////////////////////////////////////////////////////////

bool isLayerSupported( const char* layer, const vector<VkLayerProperties>& supportedLayers );
// treat layers as optional; app can always run without em -- i.e. return those supported
vector<const char*> checkInstanceLayerSupport( const vector<const char*>& requestedLayers, const vector<VkLayerProperties>& supportedLayers );
vector<VkExtensionProperties> getSupportedInstanceExtensions( const vector<const char*>& providingLayers );
bool checkExtensionSupport( const vector<const char*>& extensions, const vector<VkExtensionProperties>& supportedExtensions );

VkInstance initInstance( const vector<const char*>& layers = {}, const vector<const char*>& extensions = {} );
void killInstance( VkInstance instance );

VkPhysicalDevice getPhysicalDevice( VkInstance instance, VkSurfaceKHR surface = VK_NULL_HANDLE /*seek presentation support if !NULL*/ ); // destroyed with instance
VkPhysicalDeviceProperties getPhysicalDeviceProperties( VkPhysicalDevice physicalDevice );
VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties( VkPhysicalDevice physicalDevice );

uint32_t getQueueFamily( VkPhysicalDevice physDevice, VkSurfaceKHR surface );
vector<VkQueueFamilyProperties> getQueueFamilyProperties( VkPhysicalDevice device );

VkDevice initDevice(
	VkPhysicalDevice physDevice,
	const VkPhysicalDeviceFeatures& features,
	uint32_t queueFamilyIndex,
	const vector<const char*>& layers = {},
	const vector<const char*>& extensions = {}
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

// initSurface() is platform dependent
void killSurface( VkInstance instance, VkSurfaceKHR surface );

VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );
VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );

VkSwapchainKHR initSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSurfaceCapabilitiesKHR capabilities, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE );
void killSwapchain( VkDevice device, VkSwapchainKHR swapchain );

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


void setVertexData( VkDevice device, VkDeviceMemory memory, vector<Vertex2D_ColorF_pack> vertices );

VkSemaphore initSemaphore( VkDevice device );
void killSemaphore( VkDevice device, VkSemaphore semaphore );

VkCommandPool initCommandPool( VkDevice device, const uint32_t queueFamily );
void killCommandPool( VkDevice device, VkCommandPool commandPool );

void acquireCommandBuffers( VkDevice device, VkCommandPool commandPool, uint32_t count, vector<VkCommandBuffer>& commandBuffers );
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

void recordDraw( VkCommandBuffer commandBuffer, const vector<Vertex2D_ColorF_pack> vertices );

void submitToQueue( VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore imageReadyS, VkSemaphore renderDoneS );
void present( VkQueue queue, VkSwapchainKHR swapchain, uint32_t swapchainImageIndex, VkSemaphore renderDoneS );



// main()!
//////////////////////////////////////////////////////////////////////////////////

int main() try{
	const uint32_t vertexBufferBinding = 0;

	const float triangleSize = 1.6f;
	const vector<Vertex2D_ColorF_pack> triangle = {
		{ /*rb*/ { { 0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize} }, /*R*/{ {1.0f, 0.0f, 0.0f} }  },
		{ /* t*/ { {                0.0f, -sqrtf( 3.0f ) * 0.25f * triangleSize} }, /*G*/{ {0.0f, 1.0f, 0.0f} }  },
		{ /*lb*/ { {-0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize} }, /*B*/{ {0.0f, 0.0f, 1.0f} }  }
	};

	const auto supportedLayers = enumerate<VkLayerProperties>();
	vector<const char*> requestedLayers;
#if VULKAN_VALIDATION
	if(  isLayerSupported( "VK_LAYER_LUNARG_standard_validation", supportedLayers )  ) requestedLayers.push_back( "VK_LAYER_LUNARG_standard_validation" );
	else throw "VULKAN_VALIDATION is enabled but VK_LAYER_LUNARG_standard_validation layers are not supported!";
#endif
	if( ::fpsCounter ) requestedLayers.push_back( "VK_LAYER_LUNARG_monitor" );
	requestedLayers = checkInstanceLayerSupport( requestedLayers, supportedLayers );


	const auto supportedInstanceExtensions = getSupportedInstanceExtensions( requestedLayers );
	const auto platformSurfaceExtension = getPlatformSurfaceExtensionName();
	const vector<const char*> requestedInstanceExtensions = {
#if VULKAN_VALIDATION
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
		VK_KHR_SURFACE_EXTENSION_NAME,
		platformSurfaceExtension.c_str()
	};
	checkExtensionSupport( requestedInstanceExtensions, supportedInstanceExtensions );


	const VkInstance instance = initInstance( requestedLayers, requestedInstanceExtensions );

#if VULKAN_VALIDATION
	const VkDebugReportCallbackEXT debug = initDebug( instance, ::debugAmount );
#endif


	const PlatformWindow window = initWindow( ::initialWindowWidth, ::initialWindowHeight );
	const VkSurfaceKHR surface = initSurface( instance, window );

	const VkPhysicalDevice physicalDevice = getPhysicalDevice( instance, surface );
	const VkPhysicalDeviceProperties physicalDeviceProperties = getPhysicalDeviceProperties( physicalDevice );
	const VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = getPhysicalDeviceMemoryProperties( physicalDevice );

	const uint32_t queueFamily = getQueueFamily( physicalDevice, surface );

	const VkPhysicalDeviceFeatures features = {}; // don't need any special feature for this demo
	const vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDevice device = initDevice( physicalDevice, features, queueFamily, requestedLayers, deviceExtensions );
	VkQueue queue = getQueue( device, queueFamily, 0 );


	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat( physicalDevice, surface );
	VkRenderPass renderPass = initRenderPass( device, surfaceFormat );

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

	VkCommandPool commandPool = initCommandPool( device, queueFamily );

	// might need synchronization if init is more advanced than this
	//VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );


	// place-holder swapchain dependent objects
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	vector<VkImageView> swapchainImageViews;
	vector<VkFramebuffer> framebuffers;

	vector<VkCommandBuffer> commandBuffers;

	VkSemaphore imageReadyS = VK_NULL_HANDLE;
	VkSemaphore renderDoneS = VK_NULL_HANDLE;


	auto recreateSwapchain = [&](){
		// swapchain recreation -- will be done before the first frame too;

		VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );

		// cleanup -- BTW Vulkan is OK destroying NULL objects too
		errorCode = vkResetCommandPool( device, commandPool, 0 ); RESULT_HANDLER( errorCode, "vkResetCommandPool" );

		killSemaphore( device, imageReadyS );
		killSemaphore( device, renderDoneS );
		killFramebuffers( device, framebuffers );
		killSwapchainImageViews( device, swapchainImageViews );
		killSwapchain( device, swapchain ); swapchain = VK_NULL_HANDLE; 

		// recreation
		VkSurfaceCapabilitiesKHR capabilities = getSurfaceCapabilities( physicalDevice, surface );
		VkExtent2D surfaceSize = {
			capabilities.currentExtent.width == UINT32_MAX ? ::initialWindowWidth : capabilities.currentExtent.width,
			capabilities.currentExtent.height == UINT32_MAX ? ::initialWindowHeight : capabilities.currentExtent.height,
		};

		swapchain = initSwapchain( physicalDevice, device, surface, surfaceFormat, capabilities, swapchain );

		vector<VkImage> swapchainImages = enumerate<VkImage>( device, swapchain );
		swapchainImageViews = initSwapchainImageViews( device, swapchainImages, surfaceFormat.format );
		framebuffers = initFramebuffers( device, renderPass, swapchainImageViews, surfaceSize.width, surfaceSize.height );

		acquireCommandBuffers(  device, commandPool, static_cast<uint32_t>( swapchainImages.size() ), commandBuffers  );
		for( size_t i = 0; i < swapchainImages.size(); ++i ){
			beginCommandBuffer( commandBuffers[i] );
				recordBeginRenderPass( commandBuffers[i], renderPass, framebuffers[i], ::clearColor, surfaceSize.width, surfaceSize.height );

				recordBindPipeline( commandBuffers[i], pipeline );
				recordBindVertexBuffer( commandBuffers[i], vertexBufferBinding, vertexBuffer );

				recordSetViewport( commandBuffers[i], surfaceSize.width, surfaceSize.height );
				recordSetScissor( commandBuffers[i], surfaceSize.width, surfaceSize.height );

				recordDraw( commandBuffers[i], triangle );

				recordEndRenderPass( commandBuffers[i] );
			endCommandBuffer( commandBuffers[i] );
		}

		imageReadyS = initSemaphore( device );
		renderDoneS = initSemaphore( device );
	};


	// Finally, rendering! Yay!
	std::function<void(void)> render = [&](){
		assert( swapchain != VK_NULL_HANDLE );

		try{
			uint32_t nextSwapchainImageIndex = getNextImageIndex( device, swapchain, imageReadyS );

			submitToQueue( queue, commandBuffers[nextSwapchainImageIndex], imageReadyS, renderDoneS );
			present( queue, swapchain, nextSwapchainImageIndex, renderDoneS );
		}
		catch( VulkanResultException ex ){
			if( ex.result == VK_SUBOPTIMAL_KHR || ex.result == VK_ERROR_OUT_OF_DATE_KHR ){
				recreateSwapchain();

				// we need to start over...
				render();
			}
			else throw;
		}
	};


	setSizeEventHandler( recreateSwapchain );
	setPaintEventHandler( render );


	// Finally start the main message loop (and so render too)
	showWindow( window );
	int ret = messageLoop( window );


	// proper Vulkan cleanup
	VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );


	// kill swapchain
	killSemaphore( device, imageReadyS );
	killSemaphore( device, renderDoneS );

	killFramebuffers( device, framebuffers );

	killSwapchainImageViews( device, swapchainImageViews );
	killSwapchain( device, swapchain );

	// kill vulkan
	killCommandPool( device,  commandPool );

	killMemory( device, vertexBufferMemory );
	killBuffer( device, vertexBuffer );

	killPipeline( device, pipeline );
	killPipelineLayout( device, pipelineLayout );
	killShaderModule( device, fragmentShader );
	killShaderModule( device, vertexShader );

	killRenderPass( device, renderPass );

	killDevice( device );

	killSurface( instance, surface );
	killWindow( window );

#if VULKAN_VALIDATION
	killDebug( instance, debug );
#endif
	killInstance( instance );

	return ret;
}
catch( VulkanResultException vkE ){
	logger << "ERROR: Terminated due to an uncaught VkResult exception: "
	    << vkE.file << ":" << vkE.line << ":" << vkE.func << "() " << vkE.source << "() returned " << to_string( vkE.result )
	    << std::endl;
}
catch( const char* e ){
	logger << "ERROR: Terminated due to an uncaught exception: " << e << std::endl;
}
catch( string e ){
	logger << "ERROR: Terminated due to an uncaught exception: " << e << std::endl;
}
catch( std::exception e ){
	logger << "ERROR: Terminated due to an uncaught exception: " << e.what() << std::endl;
}
catch( ... ){
	logger << "ERROR: Terminated due to an unrecognized uncaught exception." << std::endl;
}


// Implementation
//////////////////////////////////////////////////////////////////////////////////

bool isLayerSupported( const char* layer, const vector<VkLayerProperties>& supportedLayers ){
	const auto isSupportedPred = [layer]( const VkLayerProperties& prop ) -> bool{
		return std::strcmp( layer, prop.layerName ) == 0;
	};

	return std::any_of( supportedLayers.begin(), supportedLayers.end(), isSupportedPred );
}

vector<const char*> checkInstanceLayerSupport( const vector<const char*>& requestedLayers, const vector<VkLayerProperties>& supportedLayers ){
	vector<const char*> compiledLayerList;

	for( const auto layer : requestedLayers ){
		if(  isLayerSupported( layer, supportedLayers )  ) compiledLayerList.push_back( layer );
		else logger << "WARNING: Requested layer " << layer << " is not supported. It will not be enabled." << std::endl;
	}

	return compiledLayerList;
}

vector<const char*> checkInstanceLayerSupport( const vector<const char*>& optionalLayers ){
	return checkInstanceLayerSupport( optionalLayers, enumerate<VkLayerProperties>() );
}

vector<VkExtensionProperties> getSupportedInstanceExtensions( const vector<const char*>& providingLayers ){
	auto supportedExtensions = enumerate<VkExtensionProperties>();

	for( const auto pl : providingLayers ){
		const auto providedExtensions = enumerate<VkExtensionProperties>( pl );
		supportedExtensions.insert( supportedExtensions.end(), providedExtensions.begin(), providedExtensions.end() );
	}

	return supportedExtensions;
}

vector<VkExtensionProperties> getSupportedDeviceExtensions( const VkPhysicalDevice physDevice, const vector<const char*>& providingLayers ){
	auto supportedExtensions = enumerate<VkExtensionProperties>( physDevice );

	for( const auto pl : providingLayers ){
		const auto providedExtensions = enumerate<VkExtensionProperties>( physDevice, pl );
		supportedExtensions.insert( supportedExtensions.end(), providedExtensions.begin(), providedExtensions.end() );
	}

	return supportedExtensions;
}

bool isExtensionSupported( const char* extension, const vector<VkExtensionProperties>& supportedExtensions ){
	const auto isSupportedPred = [extension]( const VkExtensionProperties& prop ) -> bool{
		return std::strcmp( extension, prop.extensionName ) == 0;
	};

	return std::any_of( supportedExtensions.begin(), supportedExtensions.end(), isSupportedPred );
}

bool checkExtensionSupport( const vector<const char*>& extensions, const vector<VkExtensionProperties>& supportedExtensions ){
	bool allSupported = true;

	for( const auto extension : extensions ){
		if(  !isExtensionSupported( extension, supportedExtensions )  ){
			allSupported = false;
			logger << "WARNING: Requested extension " << extension << " is not supported. Trying to enable it will likely fail." << std::endl;
		}
	}

	return allSupported;
}

bool checkDeviceExtensionSupport( const VkPhysicalDevice physDevice, const vector<const char*>& extensions, const vector<const char*>& providingLayers ){
	return checkExtensionSupport(  extensions, getSupportedDeviceExtensions( physDevice, providingLayers )  );
}


VkInstance initInstance( const vector<const char*>& layers, const vector<const char*>& extensions ){
	const VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr, // pNext
		u8"Hello Vulkan Triangle", // Nice to meetcha, and what's your name driver?
		0, // app version
		nullptr, // engine name
		0, // engine version
		VK_API_VERSION_1_0 // this app is written against the Vulkan 1.0 spec
	};

#if VULKAN_VALIDATION
	// in effect during vkCreateInstance and vkDestroyInstance duration (because callback object cannot be created without instance)
	const VkDebugReportCallbackCreateInfoEXT debugCreateInfo{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
		nullptr, // pNext
		::debugAmount,
		::genericDebugCallback,
		nullptr // pUserData
	};
#endif

	const VkInstanceCreateInfo instanceInfo{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if VULKAN_VALIDATION
		&debugCreateInfo,
#else
		nullptr, // pNext
#endif
		0, // flags - reserved for future use
		&appInfo,
		static_cast<uint32_t>( layers.size() ),
		layers.data(),
		static_cast<uint32_t>( extensions.size() ),
		extensions.data()
	};

	VkInstance instance;
	const VkResult errorCode = vkCreateInstance( &instanceInfo, nullptr, &instance ); RESULT_HANDLER( errorCode, "vkCreateInstance" );

	loadInstanceExtensionsCommands( instance, extensions );

	return instance;
}

void killInstance( const VkInstance instance ){
	unloadInstanceExtensionsCommands( instance );

	vkDestroyInstance( instance, nullptr );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool isPresentationSupported( const VkPhysicalDevice physDevice, const uint32_t queueFamily, const VkSurfaceKHR surface ){
	VkBool32 supported;
	const VkResult errorCode = vkGetPhysicalDeviceSurfaceSupportKHR( physDevice, queueFamily, surface, &supported ); RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfaceSupportKHR" );

	return supported == VK_TRUE;
}

bool isPresentationSupported( const VkPhysicalDevice physDevice, const VkSurfaceKHR surface ){
	uint32_t qfCount;
	vkGetPhysicalDeviceQueueFamilyProperties( physDevice, &qfCount, nullptr );

	for( uint32_t qf = 0; qf < qfCount; ++qf ){
		if(  isPresentationSupported( physDevice, qf, surface )  ) return true;
	}

	return false;
}

VkPhysicalDevice getPhysicalDevice( const VkInstance instance, const VkSurfaceKHR surface ){
	vector<VkPhysicalDevice> devices = enumerate<VkPhysicalDevice>( instance );

	if( surface ){
		for( auto it = devices.begin(); it != devices.end(); ){
			const auto& pd = *it;

			if(  !isPresentationSupported( pd, surface )  ) it = devices.erase( it );
			else ++it;
		}
	}

	if( devices.empty() ) throw string("ERROR: No Physical Devices (GPUs) ") + (surface ? "with presentation support " : "") + "detected!";
	else if( devices.size() == 1 ){
		return devices[0];
	}
	else{
		for( const auto pd : devices ){
			const VkPhysicalDeviceProperties pdp = getPhysicalDeviceProperties( pd );

			if( pdp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ){
#if VULKAN_VALIDATION
				vkDebugReportMessageEXT(
					instance, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, handleToUint64(instance), __LINE__, 
					1, u8"application", u8"More than one Physical Devices (GPU) found. Choosing the first dedicated one."
				);
#endif

				return pd;
			}
		}

#if VULKAN_VALIDATION
		vkDebugReportMessageEXT(
			instance, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, handleToUint64(instance), __LINE__, 
			1, u8"application", u8"More than one Physical Devices (GPU) found. Just choosing the first one."
		);
#endif

		return devices[0];
	}
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
	uint32_t queueFamiliesCount;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, nullptr );

	vector<VkQueueFamilyProperties> queueFamilies( queueFamiliesCount );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, queueFamilies.data() );

	return queueFamilies;
}

uint32_t getQueueFamily( const VkPhysicalDevice physDevice, const VkSurfaceKHR surface ){
	const auto qfps = getQueueFamilyProperties( physDevice );

	TODO( "Should contain the possibility of separate graphics and present queue!" )

	for( uint32_t qfi = 0; qfi < qfps.size(); ++qfi ){
		if(  (qfps[qfi].queueFlags & VK_QUEUE_GRAPHICS_BIT) && qfps[qfi].queueCount && isPresentationSupported( physDevice, qfi, surface )  ){
			return qfi;
		}
	}

	throw "Cannot find a queue family supporting both graphics + present operations!";
}

VkDevice initDevice(
	const VkPhysicalDevice physDevice,
	const VkPhysicalDeviceFeatures& features,
	const uint32_t queueFamilyIndex,
	const vector<const char*>& layers,
	const vector<const char*>& extensions
){
	checkDeviceExtensionSupport( physDevice, extensions, layers );

	const float priority[] = {1.0f};
	const VkDeviceQueueCreateInfo qci{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr, // pNext
		0, // flags
		queueFamilyIndex,
		1, // queue count
		priority
	};

	const vector<VkDeviceQueueCreateInfo> queues = { qci };

	const VkDeviceCreateInfo deviceInfo{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr, // pNext
		0, // flags
		static_cast<uint32_t>( queues.size() ),
		queues.data(),
		static_cast<uint32_t>( layers.size() ),
		layers.data(),
		static_cast<uint32_t>( extensions.size() ),
		extensions.data(),
		&features
	};


	VkDevice device;
	const VkResult errorCode = vkCreateDevice( physDevice, &deviceInfo, nullptr, &device ); RESULT_HANDLER( errorCode, "vkCreateDevice" );

	loadDeviceExtensionsCommands( device, extensions );

	return device;
}

void killDevice( const VkDevice device ){
	unloadDeviceExtensionsCommands( device );

	vkDestroyDevice( device, nullptr );
}

VkQueue getQueue( const VkDevice device, const uint32_t queueFamily, const uint32_t queueIndex ){
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
			0, // base mip-level
			1, //VK_REMAINING_MIP_LEVELS, // level count
			0, // base array layer
			1 //VK_REMAINING_ARRAY_LAYERS // array layer count
		}
	};

	TODO( "Workaround for bad layers and VK_REMAINING_* (again). Fix already in master; unfortunatelly too late for 1.0.49 SDK." )

	VkImageView imageView;
	VkResult errorCode = vkCreateImageView( device, &iciv, nullptr, &imageView ); RESULT_HANDLER( errorCode, "vkCreateImageView" );

	return imageView;
}

void killImageView( VkDevice device, VkImageView imageView ){
	vkDestroyImageView( device, imageView, nullptr );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// initSurface is platform dependent

void killSurface( VkInstance instance, VkSurfaceKHR surface ){
	vkDestroySurfaceKHR( instance, surface, nullptr );
}

VkSurfaceFormatKHR getSurfaceFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	const VkFormat preferredFormat1 = VK_FORMAT_B8G8R8A8_UNORM; 
	const VkFormat preferredFormat2 = VK_FORMAT_B8G8R8A8_SRGB;

	vector<VkSurfaceFormatKHR> formats = enumerate<VkSurfaceFormatKHR>( physicalDevice, surface );

	if( formats.empty() ) throw "No surface formats offered by Vulkan!";

	if( formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED ){
		formats[0].format = preferredFormat1;
	}

	VkSurfaceFormatKHR chosenFormat1 = {VK_FORMAT_UNDEFINED};
	VkSurfaceFormatKHR chosenFormat2 = {VK_FORMAT_UNDEFINED};

	for( auto f : formats ){
		if( f.format == preferredFormat1 ){
			chosenFormat1 = f;
			break;
		}

		if( f.format == preferredFormat2 ){
			chosenFormat2 = f;
		}
	}

	if( chosenFormat1.format ) return chosenFormat1;
	else if( chosenFormat2.format ) return chosenFormat2;
	else return formats[0];
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	VkSurfaceCapabilitiesKHR capabilities;
	VkResult errorCode = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &capabilities ); RESULT_HANDLER( errorCode, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR" );

	return capabilities;
}

int selectedMode = 0;

TODO( "Could use debug_report instead of log" )
VkPresentModeKHR getSurfacePresentMode( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	vector<VkPresentModeKHR> modes = enumerate<VkPresentModeKHR>( physicalDevice, surface );

	for( auto m : modes ){
		if( m == ::presentMode ){
			if( selectedMode != 0 ){
				logger << "INFO: Your preferred present mode became supported. Switching to it.\n";
			}

			selectedMode = 0;
			return m;
		}
	}



	for( auto m : modes ){
		if( m == VK_PRESENT_MODE_FIFO_KHR ){
			if( selectedMode != 1 ){
				logger << "WARNING: Your preferred present mode is not supported. Switching to VK_PRESENT_MODE_FIFO_KHR.\n";
			}

			selectedMode = 1;
			return m;
		}
	}

	TODO( "Workaround for bad (Intel Linux Mesa) drivers" )
	if( modes.empty() ) throw "Bugged driver reports no supported present modes.";
	else{
		if( selectedMode != 2 ){
			logger << "WARNING: Bugged drivers. VK_PRESENT_MODE_FIFO_KHR not supported. Switching to whatever is.\n";
		}

		selectedMode = 2;
		return modes[0];
	}
}

VkSwapchainKHR initSwapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSurfaceCapabilitiesKHR capabilities, VkSwapchainKHR oldSwapchain ){
	TODO( "Perhaps not really necessary to have VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR" )
	if(  !( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR )  ){
		throw "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR not supported!";
	}

	// for all modes having at least two Images can be beneficial
	TODO( "Or is it minImageCount + 1 ? Gotta think this through." )
	uint32_t minImageCount = std::max<uint32_t>( 2, capabilities.minImageCount );
	if( capabilities.maxImageCount ) minImageCount = std::min<uint32_t>( minImageCount, capabilities.maxImageCount );


	VkSwapchainCreateInfoKHR swapchainInfo{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		surface,
		minImageCount, // minImageCount
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
		oldSwapchain
	};

	VkSwapchainKHR swapchain;
	VkResult errorCode = vkCreateSwapchainKHR( device, &swapchainInfo, nullptr, &swapchain ); RESULT_HANDLER( errorCode, "vkCreateSwapchainKHR" );

	killSwapchain( device, oldSwapchain );

	return swapchain;
}

void killSwapchain( VkDevice device, VkSwapchainKHR swapchain ){
	vkDestroySwapchainKHR( device, swapchain, nullptr );
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
	imageViews.clear();
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
	framebuffers.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderModule initShaderModule( VkDevice device, string filename ){
	using std::ifstream;
	using std::istreambuf_iterator;

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
){/*
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
	};*/

	VkPipelineShaderStageCreateInfo shaderStageStates[] = { 
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr, // pNext
			0, // flags - reserved for future use
			VK_SHADER_STAGE_VERTEX_BIT,
			vertexShader,
			u8"main",
			nullptr // SpecializationInfo - constants pushed to shader on pipeline creation time
		}, 
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr, // pNext
			0, // flags - reserved for future use
			VK_SHADER_STAGE_FRAGMENT_BIT,
			fragmentShader,
			u8"main",
			nullptr // SpecializationInfo - constants pushed to shader on pipeline creation time
		}
	};

	const uint32_t vertexBufferStride = sizeof( Vertex2D_ColorF_pack );
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
		sizeof( Vertex2D_ColorF_pack ), // stride in bytes
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
	if( offsetof( Vertex2D_ColorF_pack, color ) > limits.maxVertexInputAttributeOffset ){
		throw "Implementation does not allow sufficient attribute offset.";
	}

	VkVertexInputAttributeDescription positionInputAttributeDescription{
		positionLocation,
		vertexBufferBinding,
		VK_FORMAT_R32G32_SFLOAT,
		offsetof( Vertex2D_ColorF_pack, position ) // offset in bytes
	};

	VkVertexInputAttributeDescription colorInputAttributeDescription{
		colorLocation,
		vertexBufferBinding,
		VK_FORMAT_R32G32B32_SFLOAT,
		offsetof( Vertex2D_ColorF_pack, color ) // offset in bytes
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
		1.0f // line width
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
		2, // shader stages count - vertex and fragment
		shaderStageStates,
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

void setVertexData( VkDevice device, VkDeviceMemory memory, vector<Vertex2D_ColorF_pack> vertices ){
	TODO( "Should be in Device Local memory instead" )
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

void acquireCommandBuffers( VkDevice device, VkCommandPool commandPool, uint32_t count, vector<VkCommandBuffer>& commandBuffers ){
	auto oldSize = commandBuffers.size();

	if( count > oldSize ){
		VkCommandBufferAllocateInfo commandBufferInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr, // pNext
			commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			count - static_cast<uint32_t>( oldSize ) // count
		};

		commandBuffers.resize( count );
		VkResult errorCode = vkAllocateCommandBuffers( device, &commandBufferInfo, &commandBuffers[oldSize] ); RESULT_HANDLER( errorCode, "vkAllocateCommandBuffers" );
	}
}

void beginCommandBuffer( VkCommandBuffer commandBuffer ){
	VkCommandBufferBeginInfo commandBufferInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr, // pNext
		// same buffer can be re-executed before it finishes from last submit
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
	if( width == 0 ) width = 1;
	if( height == 0 ) height = 1;

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

void recordDraw( VkCommandBuffer commandBuffer, const vector<Vertex2D_ColorF_pack> vertices ){
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
	VkPresentInfoKHR presentInfo{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr, // pNext
		1, // wait semaphore count
		&renderDoneS, // wait semaphores
		1, // swapchain count
		&swapchain,
		&swapchainImageIndex,
		nullptr // pResults
	};

	VkResult errorCode = vkQueuePresentKHR( queue, &presentInfo ); RESULT_HANDLER( errorCode, "vkQueuePresentKHR" );
}
