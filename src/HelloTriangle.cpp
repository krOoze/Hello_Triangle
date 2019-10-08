// Vulkan hello world triangle rendering demo


// Global header settings
//////////////////////////////////////////////////////////////////////////////////

#include "VulkanEnvironment.h" // first include must be before vulkan.h and platform header


// Includes
//////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <exception>
#include <fstream>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <vulkan/vulkan.h> // also assume core+WSI commands are loaded
static_assert( VK_HEADER_VERSION >= REQUIRED_HEADER_VERSION, "Update your SDK! This app is written against Vulkan header version " STRINGIZE(REQUIRED_HEADER_VERSION) "." );

#include "EnumerateScheme.h"
#include "ErrorHandling.h"
#include "ExtensionLoader.h"
#include "Vertex.h"
#include "Wsi.h"


using std::exception;
using std::runtime_error;
using std::string;
using std::to_string;
using std::vector;


// Config
//////////////////////////////////////////////////////////////////////////////////

const char appName[] = u8"Hello Vulkan Triangle";

// layers and debug
#if VULKAN_VALIDATION
	constexpr VkDebugUtilsMessageSeverityFlagsEXT debugSeverity =
		0
		//| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		//| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
	;
	constexpr VkDebugUtilsMessageTypeFlagsEXT debugType =
		0
		| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
	;

	constexpr bool useAssistantLayer = false;
#endif

constexpr bool fpsCounter = true;

// window and swapchain
constexpr uint32_t initialWindowWidth = 800;
constexpr uint32_t initialWindowHeight = 800;

//constexpr VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // better not be used often because of coil whine
constexpr VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

// pipeline settings
constexpr VkClearValue clearColor = {  { {0.1f, 0.1f, 0.1f, 1.0f} }  };


// needed stuff for main() -- forward declarations
//////////////////////////////////////////////////////////////////////////////////

bool isLayerSupported( const char* layer, const vector<VkLayerProperties>& supportedLayers );
bool isExtensionSupported( const char* extension, const vector<VkExtensionProperties>& supportedExtensions );
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
void killSwapchainImageViews( VkDevice device, vector<VkImageView>& imageViews );


VkRenderPass initRenderPass( VkDevice device, VkSurfaceFormatKHR surfaceFormat );
void killRenderPass( VkDevice device, VkRenderPass renderPass );

vector<VkFramebuffer> initFramebuffers(
	VkDevice device,
	VkRenderPass renderPass,
	vector<VkImageView> imageViews,
	uint32_t width, uint32_t height
);
void killFramebuffers( VkDevice device, vector<VkFramebuffer>& framebuffers );


VkShaderModule initShaderModule( VkDevice device, const vector<uint32_t>& shaderCode );
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
	const uint32_t vertexBufferBinding,
	uint32_t width, uint32_t height
);
void killPipeline( VkDevice device, VkPipeline pipeline );


void setVertexData( VkDevice device, VkDeviceMemory memory, vector<Vertex2D_ColorF_pack> vertices );

VkSemaphore initSemaphore( VkDevice device );
vector<VkSemaphore> initSemaphores( VkDevice device, size_t count );
void killSemaphore( VkDevice device, VkSemaphore semaphore );
void killSemaphores( VkDevice device, vector<VkSemaphore>& semaphores );

VkCommandPool initCommandPool( VkDevice device, const uint32_t queueFamily );
void killCommandPool( VkDevice device, VkCommandPool commandPool );

vector<VkFence> initFences( VkDevice device, size_t count, VkFenceCreateFlags flags = 0 );
void killFences( VkDevice device, vector<VkFence>& fences );

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

void recordDraw( VkCommandBuffer commandBuffer, uint32_t vertexCount );

void submitToQueue( VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore imageReadyS, VkSemaphore renderDoneS, VkFence fence = VK_NULL_HANDLE );
void present( VkQueue queue, VkSwapchainKHR swapchain, uint32_t swapchainImageIndex, VkSemaphore renderDoneS );



// main()!
//////////////////////////////////////////////////////////////////////////////////

int helloTriangle() try{
	const uint32_t vertexBufferBinding = 0;

	const float triangleSize = 1.6f;
	const vector<Vertex2D_ColorF_pack> triangle = {
		{ /*rb*/ { { 0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize} }, /*R*/{ {1.0f, 0.0f, 0.0f} }  },
		{ /* t*/ { {                0.0f, -sqrtf( 3.0f ) * 0.25f * triangleSize} }, /*G*/{ {0.0f, 1.0f, 0.0f} }  },
		{ /*lb*/ { {-0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize} }, /*B*/{ {0.0f, 0.0f, 1.0f} }  }
	};

	const auto supportedLayers = enumerate<VkInstance, VkLayerProperties>();
	vector<const char*> requestedLayers;

#if VULKAN_VALIDATION
	if(  isLayerSupported( "VK_LAYER_KHRONOS_validation", supportedLayers )  ) requestedLayers.push_back( "VK_LAYER_KHRONOS_validation" );
	else throw "VULKAN_VALIDATION is enabled but VK_LAYER_KHRONOS_validation layers are not supported!";

	if( ::useAssistantLayer ){
		if(  isLayerSupported( "VK_LAYER_LUNARG_assistant_layer", supportedLayers )  ) requestedLayers.push_back( "VK_LAYER_LUNARG_assistant_layer" );
		else throw "VULKAN_VALIDATION is enabled but VK_LAYER_LUNARG_assistant_layer layer is not supported!";
	}
#endif

	if( ::fpsCounter ) requestedLayers.push_back( "VK_LAYER_LUNARG_monitor" );
	requestedLayers = checkInstanceLayerSupport( requestedLayers, supportedLayers );


	const auto supportedInstanceExtensions = getSupportedInstanceExtensions( requestedLayers );
	const auto platformSurfaceExtension = getPlatformSurfaceExtensionName();
	vector<const char*> requestedInstanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		platformSurfaceExtension.c_str()
	};

#if VULKAN_VALIDATION
	DebugObjectVariant::DebugObjectTag debugExtensionTag;
	if(  isExtensionSupported( VK_EXT_DEBUG_UTILS_EXTENSION_NAME, supportedInstanceExtensions )  ){
		debugExtensionTag = DebugObjectVariant::debugUtilsType;
		requestedInstanceExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
	}
	else if(  isExtensionSupported( VK_EXT_DEBUG_REPORT_EXTENSION_NAME, supportedInstanceExtensions )  ){
		debugExtensionTag = DebugObjectVariant::debugReportType;
		requestedInstanceExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	}
	else throw "VULKAN_VALIDATION is enabled but neither VK_EXT_debug_utils nor VK_EXT_debug_report extension is supported!";
#endif

	checkExtensionSupport( requestedInstanceExtensions, supportedInstanceExtensions );


	const VkInstance instance = initInstance( requestedLayers, requestedInstanceExtensions );

#if VULKAN_VALIDATION
	const auto debugHandle = initDebug( instance, debugExtensionTag, ::debugSeverity, ::debugType );

	const int32_t uncoded = 0;
	const char* introMsg = "Validation Layers are enabled!";
	if( debugExtensionTag == DebugObjectVariant::debugUtilsType ){
		VkDebugUtilsObjectNameInfoEXT object = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			nullptr, // pNext
			VK_OBJECT_TYPE_INSTANCE,
			handleToUint64(instance),
			"instance"
		};
		const VkDebugUtilsMessengerCallbackDataEXT dumcd = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT,
			nullptr, // pNext
			0, // flags
			"VULKAN_VALIDATION", // VUID
			0, // VUID hash
			introMsg,
			0, nullptr, 0, nullptr,
			1, &object
		};
		vkSubmitDebugUtilsMessageEXT( instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &dumcd );
	}
	else if( debugExtensionTag == DebugObjectVariant::debugReportType ){
		vkDebugReportMessageEXT( instance, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, (uint64_t)instance, __LINE__, uncoded, "Application", introMsg );
	}
#endif


	const PlatformWindow window = initWindow( ::appName, ::initialWindowWidth, ::initialWindowHeight );
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

	vector<uint32_t> vertexShaderBinary = {
#include "shaders/hello_triangle.vert.spv.inl"
	};
	vector<uint32_t> fragmentShaderBinary = {
#include "shaders/hello_triangle.frag.spv.inl"
	};
	VkShaderModule vertexShader = initShaderModule( device, vertexShaderBinary );
	VkShaderModule fragmentShader = initShaderModule( device, fragmentShaderBinary );
	VkPipelineLayout pipelineLayout = initPipelineLayout( device );

	VkBuffer vertexBuffer = initBuffer( device, sizeof( decltype( triangle )::value_type ) * triangle.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );
	VkDeviceMemory vertexBufferMemory = initMemory<ResourceType::Buffer>(
		device,
		physicalDeviceMemoryProperties,
		vertexBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	setVertexData( device, vertexBufferMemory, triangle ); // Writes throug memory map. Synchronization is implicit for any subsequent vkQueueSubmit batches.

	VkCommandPool commandPool = initCommandPool( device, queueFamily );

	// might need synchronization if init is more advanced than this
	//VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );


	// place-holder swapchain dependent objects
	VkSwapchainKHR swapchain = VK_NULL_HANDLE; // has to be NULL -- signifies that there's no swapchain
	vector<VkImageView> swapchainImageViews;
	vector<VkFramebuffer> framebuffers;

	VkPipeline pipeline = VK_NULL_HANDLE; // has to be NULL for the case the app ends before even first swapchain
	vector<VkCommandBuffer> commandBuffers;

	vector<VkSemaphore> imageReadySs;
	vector<VkSemaphore> renderDoneSs;

	// workaround for validation layer "memory leak" + might also help the driver to cleanup old resources
	// this should not be needed for a real-word app, because they are likely to use fences naturaly (e.g. responding to user input )
	// read https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/1628
	const uint32_t maxInflightSubmissions = 2; // more than 2 probably does not make much sense
	uint32_t submissionNr = 0; // index of the current submission modulo maxInflightSubmission
	vector<VkFence> submissionFences;


	const std::function<bool(void)> recreateSwapchain = [&](){
		// swapchain recreation -- will be done before the first frame too;
		TODO( "This may be triggered from many sources (e.g. WM_SIZE event, and VK_ERROR_OUT_OF_DATE_KHR too). Should prevent duplicate swapchain recreation." )

		VkSurfaceCapabilitiesKHR capabilities = getSurfaceCapabilities( physicalDevice, surface );

		if( capabilities.currentExtent.width == UINT32_MAX && capabilities.currentExtent.height == UINT32_MAX ){
			capabilities.currentExtent.width = getWindowWidth( window );
			capabilities.currentExtent.height = getWindowHeight( window );
		}
		VkExtent2D surfaceSize = { capabilities.currentExtent.width, capabilities.currentExtent.height };

		const bool swapchainCreatable = {
			   surfaceSize.width >= capabilities.minImageExtent.width
			&& surfaceSize.width <= capabilities.maxImageExtent.width
			&& surfaceSize.width > 0
			&& surfaceSize.height >= capabilities.minImageExtent.height
			&& surfaceSize.height <= capabilities.maxImageExtent.height
			&& surfaceSize.height > 0
		};

		// cleanup old
		if( swapchain ){
			{VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );}

			// fences might be in unsignaled state, so kill them too to get fresh signaled
			killFences( device, submissionFences );

			// semaphores might be in signaled state, so kill them too to get fresh unsignaled
			killSemaphores( device, renderDoneSs );
			killSemaphores( device, imageReadySs );

			// only reset + later reuse already allocated and create new only if needed
			{VkResult errorCode = vkResetCommandPool( device, commandPool, 0 ); RESULT_HANDLER( errorCode, "vkResetCommandPool" );}

			killPipeline( device, pipeline );
			killFramebuffers( device, framebuffers );
			killSwapchainImageViews( device, swapchainImageViews );

			if( !swapchainCreatable ){
				// we need to wait for size that is compatible with swapchain
				// so just destroy swapchain to conserve resources in the meantime
				killSwapchain( device, swapchain );
				swapchain = VK_NULL_HANDLE;
			}
		}

		// creating new
		if( swapchainCreatable ){
			swapchain = initSwapchain( physicalDevice, device, surface, surfaceFormat, capabilities, swapchain ); // reuses & destroys oldSwapchain

			vector<VkImage> swapchainImages = enumerate<VkImage>( device, swapchain );
			swapchainImageViews = initSwapchainImageViews( device, swapchainImages, surfaceFormat.format );
			framebuffers = initFramebuffers( device, renderPass, swapchainImageViews, surfaceSize.width, surfaceSize.height );

			pipeline = initPipeline(
				device,
				physicalDeviceProperties.limits,
				pipelineLayout,
				renderPass,
				vertexShader,
				fragmentShader,
				vertexBufferBinding,
				surfaceSize.width, surfaceSize.height
			);

			acquireCommandBuffers(  device, commandPool, static_cast<uint32_t>( swapchainImages.size() ), commandBuffers  );
			for( size_t i = 0; i < swapchainImages.size(); ++i ){
				beginCommandBuffer( commandBuffers[i] );
					recordBeginRenderPass( commandBuffers[i], renderPass, framebuffers[i], ::clearColor, surfaceSize.width, surfaceSize.height );

					recordBindPipeline( commandBuffers[i], pipeline );
					recordBindVertexBuffer( commandBuffers[i], vertexBufferBinding, vertexBuffer );

					recordDraw(  commandBuffers[i], static_cast<uint32_t>( triangle.size() )  );

					recordEndRenderPass( commandBuffers[i] );
				endCommandBuffer( commandBuffers[i] );
			}

			imageReadySs = initSemaphores( device, maxInflightSubmissions );
			renderDoneSs = initSemaphores( device, maxInflightSubmissions );

			submissionFences = initFences( device, maxInflightSubmissions, VK_FENCE_CREATE_SIGNALED_BIT ); // signaled fence means previous execution finished, so we start rendering presignaled
			submissionNr = 0;
		}

		return swapchain != VK_NULL_HANDLE;
	};


	// Finally, rendering! Yay!
	const std::function<void(void)> render = [&](){
		assert( swapchain ); // should be always true; should have yielded CPU if false

		try{
			// remove oldest frame from being in flight before starting new one
			{VkResult errorCode = vkWaitForFences( device, 1, &submissionFences[submissionNr], VK_TRUE, UINT64_MAX ); RESULT_HANDLER( errorCode, "vkWaitForFences" );}
			{VkResult errorCode = vkResetFences( device, 1, &submissionFences[submissionNr] ); RESULT_HANDLER( errorCode, "vkResetFences" );}

			uint32_t nextSwapchainImageIndex = getNextImageIndex( device, swapchain, imageReadySs[submissionNr] );
			submitToQueue( queue, commandBuffers[nextSwapchainImageIndex], imageReadySs[submissionNr], renderDoneSs[submissionNr], submissionFences[submissionNr] );
			present( queue, swapchain, nextSwapchainImageIndex, renderDoneSs[submissionNr] );

			submissionNr = (submissionNr + 1) % maxInflightSubmissions;
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
	int exitStatus = messageLoop( window );


	// proper Vulkan cleanup
	VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );


	// kill swapchain
	killSemaphores( device, renderDoneSs );
	killSemaphores( device, imageReadySs );

	// command buffers killed with pool

	killPipeline( device, pipeline );

	killFramebuffers( device, framebuffers );

	killSwapchainImageViews( device, swapchainImageViews );
	killSwapchain( device, swapchain );


	// kill vulkan
	killFences( device, submissionFences );

	killCommandPool( device,  commandPool );

	killMemory( device, vertexBufferMemory );
	killBuffer( device, vertexBuffer );

	killPipelineLayout( device, pipelineLayout );
	killShaderModule( device, fragmentShader );
	killShaderModule( device, vertexShader );

	killRenderPass( device, renderPass );

	killDevice( device );

	killSurface( instance, surface );
	killWindow( window );

#if VULKAN_VALIDATION
	killDebug( instance, debugHandle );
#endif
	killInstance( instance );

	return exitStatus;
}
catch( VulkanResultException vkE ){
	logger << "ERROR: Terminated due to an uncaught VkResult exception: "
	       << vkE.file << ":" << vkE.line << ":" << vkE.func << "() " << vkE.source << "() returned " << to_string( vkE.result )
	       << std::endl;
	return EXIT_FAILURE;
}
catch( const char* e ){
	logger << "ERROR: Terminated due to an uncaught exception: " << e << std::endl;
	return EXIT_FAILURE;
}
catch( string e ){
	logger << "ERROR: Terminated due to an uncaught exception: " << e << std::endl;
	return EXIT_FAILURE;
}
catch( std::exception e ){
	logger << "ERROR: Terminated due to an uncaught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch( ... ){
	logger << "ERROR: Terminated due to an unrecognized uncaught exception." << std::endl;
	return EXIT_FAILURE;
}


#if defined(_WIN32) && !defined(_CONSOLE)
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int ){
	return helloTriangle();
}
#else
int main(){
	return helloTriangle();
}
#endif

// Implementation
//////////////////////////////////////////////////////////////////////////////////

bool isLayerSupported( const char* layer, const vector<VkLayerProperties>& supportedLayers ){
	const auto isSupportedPred = [layer]( const VkLayerProperties& prop ) -> bool{
		return std::strcmp( layer, prop.layerName ) == 0;
	};

	return std::any_of( supportedLayers.begin(), supportedLayers.end(), isSupportedPred );
}

bool isExtensionSupported( const char* extension, const vector<VkExtensionProperties>& supportedExtensions ){
	const auto isSupportedPred = [extension]( const VkExtensionProperties& prop ) -> bool{
		return std::strcmp( extension, prop.extensionName ) == 0;
	};

	return std::any_of( supportedExtensions.begin(), supportedExtensions.end(), isSupportedPred );
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
	return checkInstanceLayerSupport( optionalLayers, enumerate<VkInstance, VkLayerProperties>() );
}

vector<VkExtensionProperties> getSupportedInstanceExtensions( const vector<const char*>& providingLayers ){
	auto supportedExtensions = enumerate<VkInstance, VkExtensionProperties>();

	for( const auto pl : providingLayers ){
		const auto providedExtensions = enumerate<VkInstance, VkExtensionProperties>( pl );
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
		::appName, // Nice to meetcha, and what's your name driver?
		0, // app version
		nullptr, // engine name
		0, // engine version
		VK_API_VERSION_1_0 // this app is written against the Vulkan 1.0 spec
	};

#if VULKAN_VALIDATION
	// in effect during vkCreateInstance and vkDestroyInstance duration (because callback object cannot be created without instance)
	const VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
		nullptr, // pNext
		translateFlags( ::debugSeverity, ::debugType ),
		::genericDebugReportCallback,
		nullptr // pUserData
	};

	const VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr, // pNext
		0, // flags
		debugSeverity,
		debugType,
		::genericDebugUtilsCallback,
		nullptr // pUserData
	};

	bool debugUtils = std::find_if( extensions.begin(), extensions.end(), [](const char* e){ return std::strcmp( e, VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) == 0; } ) != extensions.end();
	bool debugReport = std::find_if( extensions.begin(), extensions.end(), [](const char* e){ return std::strcmp( e, VK_EXT_DEBUG_REPORT_EXTENSION_NAME ) == 0; } ) != extensions.end();
	if( !debugUtils && !debugReport ) throw "VULKAN_VALIDATION is enabled but neither VK_EXT_debug_utils nor VK_EXT_debug_report extension is being enabled!";
	const void* debugpNext = debugUtils ? (void*)&debugUtilsCreateInfo : (void*)&debugReportCreateInfo;
#endif

	const VkInstanceCreateInfo instanceInfo{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if VULKAN_VALIDATION
		debugpNext,
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

	const vector<VkDeviceQueueCreateInfo> queues = {
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr, // pNext
			0, // flags
			queueFamilyIndex,
			1, // queue count
			priority
		}
	};

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
			VK_REMAINING_MIP_LEVELS, // level count
			0, // base array layer
			VK_REMAINING_ARRAY_LAYERS // array layer count
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
	// we don't care as we are always setting alpha to 1.0
	VkCompositeAlphaFlagBitsKHR compositeAlphaFlag;
	if( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR ) compositeAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	else if( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR ) compositeAlphaFlag = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	else if( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR ) compositeAlphaFlag = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	else if( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR ) compositeAlphaFlag = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	else throw "Unknown composite alpha reported.";

	// minImageCount + 1 seems a sensible default. It means 2 images should always be readily available without blocking. May lead to memory waste though if we care about that.
	uint32_t myMinImageCount = capabilities.minImageCount + 1;
	if( capabilities.maxImageCount ) myMinImageCount = std::min<uint32_t>( myMinImageCount, capabilities.maxImageCount );


	VkSwapchainCreateInfoKHR swapchainInfo{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr, // pNext for extensions use
		0, // flags - reserved for future use
		surface,
		myMinImageCount, // minImageCount
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		capabilities.currentExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // VkImage usage flags
		VK_SHARING_MODE_EXCLUSIVE,
		0, // sharing queue families count
		nullptr, // sharing queue families
		capabilities.currentTransform,
		compositeAlphaFlag,
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

void killSwapchainImageViews( VkDevice device, vector<VkImageView>& imageViews ){
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
		0, // srcAccessMask
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // dstAccessMask
		VK_DEPENDENCY_BY_REGION_BIT, // dependencyFlags
	};

	// implicitly defined dependency would cover this, but let's replace it with this explicitly defined dependency!
	VkSubpassDependency dstDependency{
		0, // srcSubpass
		VK_SUBPASS_EXTERNAL, // dstSubpass
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // srcAccessMask
		0, // dstAccessMask
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

void killFramebuffers( VkDevice device, vector<VkFramebuffer>& framebuffers ){
	for( auto framebuffer : framebuffers ) vkDestroyFramebuffer( device, framebuffer, nullptr );
	framebuffers.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Type = uint8_t>
vector<Type> loadBinaryFile( string filename ){
	using std::ifstream;
	using std::istreambuf_iterator;

	vector<Type> data;

	try{
		ifstream ifs;
		ifs.exceptions( ifs.failbit | ifs.badbit | ifs.eofbit );
		ifs.open( filename, ifs.in | ifs.binary | ifs.ate );

		const auto fileSize = static_cast<size_t>( ifs.tellg() );

		if( fileSize > 0 && (fileSize % sizeof(Type) == 0) ){
			ifs.seekg( ifs.beg );
			data.resize( fileSize / sizeof(Type) );
			ifs.read( reinterpret_cast<char*>(data.data()), fileSize );
		}
	}
	catch( ... ){
		data.clear();
	}

	return data;
}

VkShaderModule initShaderModule( VkDevice device, string filename ){
	const auto shaderCode = loadBinaryFile<uint32_t>( filename );
	if( shaderCode.empty() ) throw "SPIR-V shader file " + filename + " is invalid or read failed!";
	return initShaderModule( device, shaderCode );
}

VkShaderModule initShaderModule( VkDevice device, const vector<uint32_t>& shaderCode ){
	VkShaderModuleCreateInfo shaderModuleInfo{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		shaderCode.size() * sizeof(uint32_t),
		shaderCode.data()
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
	const uint32_t vertexBufferBinding,
	uint32_t width, uint32_t height
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

	VkViewport viewport{
		0.0f, // x
		0.0f, // y
		static_cast<float>( width ? width : 1 ),
		static_cast<float>( height ? height : 1 ),
		0.0f, // min depth
		1.0f // max depth
	};

	VkRect2D scissor{
		{0, 0}, // offset
		{width, height}
	};

	VkPipelineViewportStateCreateInfo viewportState{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		1, // Viewport count
		&viewport,
		1, // scisor count,
		&scissor
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
		nullptr, // dynamic state
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
	const VkSemaphoreCreateInfo semaphoreInfo{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr, // pNext
		0 // flags - reserved for future use
	};

	VkSemaphore semaphore;
	VkResult errorCode = vkCreateSemaphore( device, &semaphoreInfo, nullptr, &semaphore ); RESULT_HANDLER( errorCode, "vkCreateSemaphore" );
	return semaphore;
}

vector<VkSemaphore> initSemaphores( VkDevice device, size_t count ){
	vector<VkSemaphore> semaphores;
	std::generate_n(  std::back_inserter( semaphores ), count, [device]{ return initSemaphore( device ); }  );
	return semaphores;
}

void killSemaphore( VkDevice device, VkSemaphore semaphore ){
	vkDestroySemaphore( device, semaphore, nullptr );
}

void killSemaphores( VkDevice device, vector<VkSemaphore>& semaphores ){
	for( const auto s : semaphores ) killSemaphore( device, s );
	semaphores.clear();
}

VkCommandPool initCommandPool( VkDevice device, const uint32_t queueFamily ){
	const VkCommandPoolCreateInfo commandPoolInfo{
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

VkFence initFence( const VkDevice device, const VkFenceCreateFlags flags = 0 ){
	const VkFenceCreateInfo fci{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr, // pNext
		flags
	};

	VkFence fence;
	VkResult errorCode = vkCreateFence( device, &fci, nullptr, &fence ); RESULT_HANDLER( errorCode, "vkCreateFence" );
	return fence;
}

void killFence( const VkDevice device, const VkFence fence ){
	vkDestroyFence( device, fence, nullptr );
}

vector<VkFence> initFences( const VkDevice device, const size_t count, const VkFenceCreateFlags flags ){
	vector<VkFence> fences;
	std::generate_n(  std::back_inserter( fences ), count, [=]{return initFence( device, flags );}  );
	return fences;
}

void killFences( const VkDevice device, vector<VkFence>& fences ){
	for( const auto f : fences ) killFence( device, f );
	fences.clear();
}

void acquireCommandBuffers( VkDevice device, VkCommandPool commandPool, uint32_t count, vector<VkCommandBuffer>& commandBuffers ){
	const auto oldSize = static_cast<uint32_t>( commandBuffers.size() );

	if( count > oldSize ){
		VkCommandBufferAllocateInfo commandBufferInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr, // pNext
			commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			count - oldSize // count
		};

		commandBuffers.resize( count );
		VkResult errorCode = vkAllocateCommandBuffers( device, &commandBufferInfo, &commandBuffers[oldSize] ); RESULT_HANDLER( errorCode, "vkAllocateCommandBuffers" );
	}

	if( count < oldSize ) {
		vkFreeCommandBuffers( device, commandPool, oldSize - count, &commandBuffers[count] );
		commandBuffers.resize( count );
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

void recordDraw( VkCommandBuffer commandBuffer, const uint32_t vertexCount ){
	vkCmdDraw( commandBuffer, vertexCount, 1 /*instance count*/, 0 /*first vertex*/, 0 /*first instance*/ );
}

void submitToQueue( VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore imageReadyS, VkSemaphore renderDoneS, VkFence fence ){
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

	VkResult errorCode = vkQueueSubmit( queue, 1 /*submit count*/, &submit, fence ); RESULT_HANDLER( errorCode, "vkQueueSubmit" );
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
