// Vulkan hello world triangle rendering demo with compute and queue family transition


// Global header settings
//////////////////////////

#define _CRT_SECURE_NO_WARNINGS

#include "LeanWindowsEnvironment.h"
#include "VulkanEnvironment.h"


// Includes
//////////////////////////

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

#include <algorithm>

#include <vulkan/vulkan.h>

#include "ErrorHandling.h"
#include "Vertex.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
	#include "win32Platform.h"
#endif

// Config
///////////////////////

// layers and debug
TODO( "Should be also guarded by VK_EXT_debug_report" )
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
const char* computeShaderFilename = "compute.comp.spv";

// needed stuff -- forward declarations
///////////////////////////////

VkInstance initInstance( const vector<const char*> layers = {}, const vector<const char*> extensions = {} );
void killInstance( VkInstance instance );

VkPhysicalDevice getPhysicalDevice( VkInstance instance ); // destroyed with instance
VkPhysicalDeviceProperties getPhysicalDeviceProperties( VkPhysicalDevice physicalDevice );
VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties( VkPhysicalDevice physicalDevice );

uint32_t getQueueFamily( VkPhysicalDevice physDevice );
uint32_t getDedicatedComputeQueueFamily( VkPhysicalDevice physDevice );

VkDevice initDevice(
	VkPhysicalDevice physDevice,
	const VkPhysicalDeviceFeatures& features,
	vector<uint32_t> queueFamilies,
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

// initSurface() is platform dependent
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


VkDescriptorSetLayout initDescriptorSetLayout( VkDevice device, vector<VkDescriptorSetLayoutBinding> bindings );
void killDescriptorSetLayout( VkDevice device, VkDescriptorSetLayout descriptorSetLayout );

VkDescriptorPool initDescriptorPool( VkDevice device, uint32_t maxSets, VkDescriptorType type, uint32_t maxDesc );
void killDescriptorPool( VkDevice device, VkDescriptorPool descriptorPool );

vector<VkDescriptorSet> acquireDescriptorSets( VkDevice device, VkDescriptorPool descriptorPool, vector<VkDescriptorSetLayout> descriptorSetLayouts );
void updateDescriptorSet( VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, VkImageView imageView, VkImageLayout expectedLayout );
void recordBindDescriptorSet(
	VkCommandBuffer commandBuffer,
	VkPipelineBindPoint bindPoint,
	VkPipelineLayout pipelineLayout,
	vector<VkDescriptorSet> descriptorSets
);

VkShaderModule initShaderModule( VkDevice device, string filename );
void killShaderModule( VkDevice device, VkShaderModule shaderModule );

VkPipelineLayout initPipelineLayout( VkDevice device, vector<VkDescriptorSetLayout> descriptorSetLayouts = {} );
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
VkPipeline initComputePipeline( VkDevice device, VkPipelineLayout pipelineLayout, VkShaderModule computeShader );
void killPipeline( VkDevice device, VkPipeline pipeline );


void setVertexData( VkDevice device, VkDeviceMemory memory, vector<Vertex2D_ColorF_pack> vertices );

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

void recordBindPipeline( VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, VkPipeline pipeline );
void recordBindVertexBuffer( VkCommandBuffer commandBuffer, const uint32_t vertexBufferBinding, VkBuffer vertexBuffer );

void recordSetViewport( VkCommandBuffer commandBuffer, uint32_t width, uint32_t height );
void recordSetScissor( VkCommandBuffer commandBuffer, uint32_t width, uint32_t height );

void recordDraw( VkCommandBuffer commandBuffer, const vector<Vertex2D_ColorF_pack> vertices );

void recordImageBarrier(
	VkCommandBuffer commandBuffer,
	VkImage image,
	VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage,
	VkAccessFlags srcAccess,
	VkAccessFlags dstAccess,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t oldQueueFamily = VK_QUEUE_FAMILY_IGNORED,
	uint32_t newQueueFamily = VK_QUEUE_FAMILY_IGNORED
);

void submitToQueue( VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore waitS, VkPipelineStageFlags waitStage, VkSemaphore signalS );
void present( VkQueue queue, VkSwapchainKHR swapchain, uint32_t swapchainImageIndex, VkSemaphore renderDoneS );



// main()!
////////////////////////

int main() try{
	const uint32_t vertexBufferBinding = 0;
	const uint32_t computeImageBinding = 0;

	const float triangleSize = 1.6f;
	vector<Vertex2D_ColorF_pack> triangle = {
		{ /*rb*/ {  0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize }, /*R*/{ 1.0f, 0.0f, 0.0f }  },
		{ /* t*/ {                 0.0f, -sqrtf( 3.0f ) * 0.25f * triangleSize }, /*G*/{ 0.0f, 1.0f, 0.0f }  },
		{ /*lb*/ { -0.5f * triangleSize,  sqrtf( 3.0f ) * 0.25f * triangleSize }, /*B*/{ 0.0f, 0.0f, 1.0f }  }
	};

	vector<const char*> layers;
	if( ::debugVulkan ) layers.push_back( "VK_LAYER_LUNARG_standard_validation" );

	vector<const char*> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME, PLATFORM_SURFACE_EXTENSION_NAME};
	if( ::debugVulkan ) instanceExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

	VkInstance instance = initInstance(
		layers,
		instanceExtensions
	);

	VkDebugReportCallbackEXT debug = ::debugVulkan ? initDebug( instance, ::debugAmount ) : VK_NULL_HANDLE;

	VkPhysicalDevice physicalDevice = getPhysicalDevice( instance );

	VkPhysicalDeviceFeatures features = {};
	features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
	VkPhysicalDeviceProperties physicalDeviceProperties = getPhysicalDeviceProperties( physicalDevice );
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = getPhysicalDeviceMemoryProperties( physicalDevice );
	uint32_t queueFamily = getQueueFamily( physicalDevice );
	uint32_t computeQueueFamily = getDedicatedComputeQueueFamily( physicalDevice );
	vector<uint32_t> families = { queueFamily, computeQueueFamily };
	VkDevice device = initDevice(
		physicalDevice,
		features,
		families,
		layers,
		{VK_KHR_SWAPCHAIN_EXTENSION_NAME}
	);
	VkQueue queue = getQueue( device, queueFamily, 0 );
	VkQueue computeQueue = getQueue( device, computeQueueFamily, 0 );

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
	const uint32_t imageCount = static_cast<uint32_t>( swapchainImages.size() );

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

	VkDescriptorSetLayoutBinding computeDescriptorSetLayoutBinding{
		computeImageBinding,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		1, // descriptorCount
		VK_SHADER_STAGE_COMPUTE_BIT,
		nullptr // pImmutableSamplers -- ignored without samplers
	};
	VkDescriptorSetLayout computeDescriptorSetLayout = initDescriptorSetLayout( device, {computeDescriptorSetLayoutBinding} );
	VkDescriptorPool computeDescriptorPool = initDescriptorPool( device, imageCount, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, imageCount );
	vector<VkDescriptorSet> computeDescriptorSets = acquireDescriptorSets(
		device,
		computeDescriptorPool,
		vector<VkDescriptorSetLayout>( imageCount, computeDescriptorSetLayout )
	);

	VkShaderModule computeShader = initShaderModule( device, ::computeShaderFilename );
	VkPipelineLayout computePipelineLayout = initPipelineLayout( device, {computeDescriptorSetLayout} );
	VkPipeline computePipeline = initComputePipeline( device, computePipelineLayout, computeShader );

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
	VkSemaphore computeDoneS = initSemaphore( device );
	VkSemaphore transferDoneS = initSemaphore( device );


	VkCommandPool commandPool = initCommandPool( device, queueFamily );
	VkCommandPool computeCommandPool = initCommandPool( device, computeQueueFamily );


	vector<VkCommandBuffer> commandBuffers = acquireCommandBuffers( device, commandPool, imageCount );
	for( size_t i = 0; i < commandBuffers.size(); ++i ){
		beginCommandBuffer( commandBuffers[i] );
			recordBeginRenderPass( commandBuffers[i], renderPass, framebuffers[i], ::clearColor, ::windowWidth, ::windowHeight );

			recordBindPipeline( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
			recordBindVertexBuffer( commandBuffers[i], vertexBufferBinding, vertexBuffer );

			recordSetViewport( commandBuffers[i], ::windowWidth, ::windowHeight );
			recordSetScissor( commandBuffers[i], ::windowWidth, ::windowHeight );

			recordDraw( commandBuffers[i], triangle );

			recordEndRenderPass( commandBuffers[i] );

			recordImageBarrier(
				commandBuffers[i],
				swapchainImages[i],
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
				queueFamily, computeQueueFamily
			);
		endCommandBuffer( commandBuffers[i] );
	}


	vector<VkCommandBuffer> computeCommandBuffers = acquireCommandBuffers( device, computeCommandPool, imageCount );
	for( size_t i = 0; i < computeCommandBuffers.size(); ++i ){
		updateDescriptorSet( device, computeDescriptorSets[i], computeImageBinding, swapchainImageViews[i], VK_IMAGE_LAYOUT_GENERAL );

		beginCommandBuffer( computeCommandBuffers[i] );
			recordBindPipeline( computeCommandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline );
			recordBindDescriptorSet( computeCommandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, {computeDescriptorSets[i]} );


			recordImageBarrier(
				computeCommandBuffers[i],
				swapchainImages[i],
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				/*VK_IMAGE_LAYOUT_GENERAL*/VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
				queueFamily, computeQueueFamily
			);

			vkCmdDispatch( computeCommandBuffers[i], ::windowWidth, ::windowHeight, 1 );

			recordImageBarrier(
				computeCommandBuffers[i],
				swapchainImages[i],
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
				VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				computeQueueFamily, queueFamily
			);
		endCommandBuffer( computeCommandBuffers[i] );
	}

	// NOTE: might not be needed (waiting on response from spec makers)
	vector<VkCommandBuffer> transferBackCommandBuffers = acquireCommandBuffers( device, commandPool, imageCount );
	for( size_t i = 0; i < computeCommandBuffers.size(); ++i ){
		beginCommandBuffer( transferBackCommandBuffers[i] );
			recordImageBarrier(
				transferBackCommandBuffers[i],
				swapchainImages[i],
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
				/*VK_IMAGE_LAYOUT_PRESENT_SRC_KHR*/ VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				computeQueueFamily, queueFamily
			);
		endCommandBuffer( transferBackCommandBuffers[i] );
	}


	// might need synchronization if init is more advanced than this
	//VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );

	// lets have simple non-robust performance info for fun
	unsigned frames = 0;
	steady_clock::time_point start = steady_clock::now();

	int ret = EXIT_SUCCESS;
	for( bool quit = false; !quit; ){
		ret = messageLoop( quit ); // process all available events

		// Rendering! Yay!
		uint32_t nextSwapchainImageIndex = getNextImageIndex( device, swapchain, imageReadyS );
		submitToQueue( queue, commandBuffers[nextSwapchainImageIndex], imageReadyS, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderDoneS );
		submitToQueue( computeQueue, computeCommandBuffers[nextSwapchainImageIndex], renderDoneS, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, computeDoneS );
		// NOTE: might not be needed (waiting on response from spec makers)
		submitToQueue( queue, transferBackCommandBuffers[nextSwapchainImageIndex], computeDoneS, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, transferDoneS );
		present( queue, swapchain, nextSwapchainImageIndex, transferDoneS );
		++frames;
	}

	VkResult errorCode = vkDeviceWaitIdle( device ); RESULT_HANDLER( errorCode, "vkDeviceWaitIdle" );

	steady_clock::time_point end = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>( end - start );
	cout << "Rendered " << frames << " frames in " << time_span.count() << " seconds. Average FPS is " << frames / time_span.count() << endl;

	killCommandPool( device, computeCommandPool );
	killCommandPool( device, commandPool );

	killSemaphore( device, imageReadyS );
	killSemaphore( device, renderDoneS );
	killSemaphore( device, computeDoneS );
	killSemaphore( device, transferDoneS );

	killMemory( device, vertexBufferMemory );
	killBuffer( device, vertexBuffer );

	killPipeline( device, computePipeline );
	killPipelineLayout( device, computePipelineLayout );
	killShaderModule( device, computeShader );

	killDescriptorPool( device, computeDescriptorPool );
	killDescriptorSetLayout( device, computeDescriptorSetLayout );

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


// Implementation
//////////////////////////////////

void killSurface( VkInstance instance, VkSurfaceKHR surface ){
	vkDestroySurfaceKHR( instance, surface, nullptr );
}

VkInstance initInstance( const vector<const char*> layers, const vector<const char*> extensions ){
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = u8"Hello Vulkan Triangle 3: Resource Sharing";
	appInfo.apiVersion = 0; // 0 should accept any version

	TODO( "Should make a warning if Vulkan version is not the expected 1.0" )

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
		genericDebugCallback,
		nullptr
	};

	if( ::debugVulkan ){
		instanceInfo.pNext = &debugCreateInfo; // valid just for createInstance/destroyInstance execution
	}


	VkInstance instance;
	VkResult errorCode = vkCreateInstance( &instanceInfo, nullptr, &instance ); RESULT_HANDLER( errorCode, "vkCreateInstance" );

	TODO( "On layer and extension failure should enumerate them and give useful error message" )

	return instance;
}

void killInstance( VkInstance instance ){
	vkDestroyInstance( instance, nullptr );
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

	TODO( "Should try to pick the best, not first" )
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

	for( ; qfi < qfps.size(); ++qfi ){
		if( qfps[qfi].queueFlags & VK_QUEUE_GRAPHICS_BIT ){
			TODO( "Might need some platform dependent stuff passed" )
			if(  presentationSupport( physDevice, qfi )  ){
				return qfi;
			}
		}
	}

	throw "Can't find a queue family supporting both graphics + present operations!";
}

uint32_t getDedicatedComputeQueueFamily( VkPhysicalDevice physDevice ){
	auto qfps = getQueueFamilyProperties( physDevice );
	uint32_t qfi = 0;

	for( ; qfi < qfps.size(); ++qfi ){
		if( (qfps[qfi].queueFlags & VK_QUEUE_COMPUTE_BIT) && !(qfps[qfi].queueFlags & VK_QUEUE_GRAPHICS_BIT) ){
			return qfi;
		}
	}

	throw "Can't find a dedicated queue family supporting compute operations!";
}

VkDevice initDevice(
	VkPhysicalDevice physDevice,
	const VkPhysicalDeviceFeatures& features,
	vector<uint32_t> queueFamilies,
	vector<const char*> layers,
	vector<const char*> extensions
){
	vector<VkDeviceQueueCreateInfo> queues;
	for( auto queueFamilyIndex : queueFamilies ){
		const float priority[] = {1.0f};
		VkDeviceQueueCreateInfo qci{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			queueFamilyIndex,
			1,
			priority
		};

		queues.push_back( qci );
	}

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

	if(  !( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT )  ){
		throw "VK_IMAGE_USAGE_STORAGE_BIT not supported!";
	}

	TODO( "Perhaps not really necessary to have VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR" )
	if(  !( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR )  ){
		throw "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR not supported!";
	}

	// for all modes having at least two Images can be beneficial
	uint32_t minImageCount = std::min<uint32_t>(
		capabilities.maxImageCount,
		std::max<uint32_t>(
			2,
			capabilities.minImageCount
		)
	);


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
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT, // VkImage usage flags
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
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL//VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
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
		0 //VK_DEPENDENCY_BY_REGION_BIT, // dependencyFlags
	};
	/*
	VkSubpassDependency dstDependency{
		0, // srcSubpass
		VK_SUBPASS_EXTERNAL, // dstSubpass
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // srcAccessMask
		VK_ACCESS_MEMORY_READ_BIT, // dstAccessMask
		VK_DEPENDENCY_BY_REGION_BIT, // dependencyFlags
	};*/

	vector<VkSubpassDependency> dependencies = {srcDependency/*, dstDependency*/};

	VkRenderPassCreateInfo renderPassInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		1, // attachment count
		&colorAtachment, // attachments
		1, // subpass count
		&subpass, // subpasses
		static_cast<uint32_t>( dependencies.size() ), // dependency count
		dependencies.data() // dependencies
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

VkDescriptorSetLayout initDescriptorSetLayout( VkDevice device, vector<VkDescriptorSetLayoutBinding> bindings ){
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr, // pNext
		0, // flags -- reserved for future use
		static_cast<uint32_t>( bindings.size() ),
		bindings.data()
	};

	VkDescriptorSetLayout descriptorSetLayout;
	VkResult errorCode = vkCreateDescriptorSetLayout( device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout ); RESULT_HANDLER( errorCode, "vkCreateDescriptorSetLayout" );
	return descriptorSetLayout;
}

void killDescriptorSetLayout( VkDevice device, VkDescriptorSetLayout descriptorSetLayout ){
	vkDestroyDescriptorSetLayout( device, descriptorSetLayout, nullptr );
}

VkDescriptorPool initDescriptorPool( VkDevice device, uint32_t maxSets, VkDescriptorType type, uint32_t maxDesc ){
	VkDescriptorPoolSize poolSize{ type, maxDesc };
	VkDescriptorPoolCreateInfo descriptorPoolInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr, // pNext
		0, // flags
		maxSets,
		1,
		&poolSize
	};

	VkDescriptorPool descriptorPool;
	VkResult errorCode = vkCreateDescriptorPool( device, &descriptorPoolInfo, nullptr, &descriptorPool ); RESULT_HANDLER( errorCode, "vkCreateDescriptorPool" );
	return descriptorPool;
}

void killDescriptorPool( VkDevice device, VkDescriptorPool descriptorPool ){
	vkDestroyDescriptorPool( device, descriptorPool, nullptr );
}

vector<VkDescriptorSet> acquireDescriptorSets( VkDevice device, VkDescriptorPool descriptorPool, vector<VkDescriptorSetLayout> descriptorSetLayouts ){
	VkDescriptorSetAllocateInfo descriptorSetInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr, // pNext
		descriptorPool,
		static_cast<uint32_t>( descriptorSetLayouts.size() ),
		descriptorSetLayouts.data()
	};

	vector<VkDescriptorSet> descriptorSets( descriptorSetLayouts.size() );
	VkResult errorCode = vkAllocateDescriptorSets( device, &descriptorSetInfo, descriptorSets.data() ); RESULT_HANDLER( errorCode, "vkAllocateDescriptorSets" );
	return descriptorSets;
}

void updateDescriptorSet( VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, VkImageView imageView, VkImageLayout expectedLayout ){
	VkDescriptorImageInfo imageInfo{
		VK_NULL_HANDLE, // sampler
		imageView,
		expectedLayout
	};

	VkWriteDescriptorSet descriptorWrite{
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		nullptr, // pNext
		descriptorSet,
		binding,
		0, // starting array element
		1, // descroptor count
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		&imageInfo,
		nullptr, // buffers
		nullptr // bufferViews
	};

	vkUpdateDescriptorSets( device, 1, &descriptorWrite, 0, nullptr );
}

void recordBindDescriptorSet(
	VkCommandBuffer commandBuffer,
	VkPipelineBindPoint bindPoint,
	VkPipelineLayout pipelineLayout,
	vector<VkDescriptorSet> descriptorSets
){
	vkCmdBindDescriptorSets(
		commandBuffer,
		bindPoint,
		pipelineLayout,
		0, // first set
		static_cast<uint32_t>( descriptorSets.size() ), descriptorSets.data(),
		0, nullptr // dynamic offsets
	);
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

VkPipelineLayout initPipelineLayout( VkDevice device, vector<VkDescriptorSetLayout> descriptorSetLayouts ){
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		static_cast<uint32_t>( descriptorSetLayouts.size() ), // descriptorSetLayout count
		descriptorSetLayouts.data(),
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

VkPipeline initComputePipeline( VkDevice device, VkPipelineLayout pipelineLayout, VkShaderModule computeShader ){
	const VkPipelineShaderStageCreateInfo computeShaderStage{
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - reserved for future use
		VK_SHADER_STAGE_COMPUTE_BIT,
		computeShader,
		u8"main",
		nullptr // SpecializationInfo - constants pushed to shader on pipeline creation time
	};

	VkComputePipelineCreateInfo pipelineInfo{
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		nullptr, // pNext
		0, // flags - e.g. disable optimization
		computeShaderStage,
		pipelineLayout,
		VK_NULL_HANDLE, // base pipeline
		-1 // base pipeline index
	};

	VkPipeline pipeline;
	VkResult errorCode = vkCreateComputePipelines(
		device,
		VK_NULL_HANDLE /* pipeline cache */,
		1 /* info count */,
		&pipelineInfo,
		nullptr,
		&pipeline
	); RESULT_HANDLER( errorCode, "vkCreateComputePipelines" );
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

void recordBindPipeline( VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, VkPipeline pipeline ){
	vkCmdBindPipeline( commandBuffer, bindPoint, pipeline );
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

void recordDraw( VkCommandBuffer commandBuffer, const vector<Vertex2D_ColorF_pack> vertices ){
	vkCmdDraw( commandBuffer, static_cast<uint32_t>( vertices.size() ), 1 /*instance count*/, 0 /*first vertex*/, 0 /*first instance*/ );
}

void recordImageBarrier(
	VkCommandBuffer commandBuffer,
	VkImage image,
	VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage,
	VkAccessFlags srcAccess,
	VkAccessFlags dstAccess,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t oldQueueFamily,
	uint32_t newQueueFamily
){
	VkImageSubresourceRange range{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, VK_REMAINING_MIP_LEVELS,
		0, VK_REMAINING_ARRAY_LAYERS
	};

	VkImageMemoryBarrier imageBarrier{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr, // pNext
		srcAccess,
		dstAccess,
		oldLayout,
		newLayout,
		oldQueueFamily,
		newQueueFamily,
		image,
		range
	};

	vkCmdPipelineBarrier(
		commandBuffer, srcStage, dstStage, 0 /*or VK_DEPENDENCY_BY_REGION_BIT*/,
		0, nullptr,
		0, nullptr,
		1, &imageBarrier
	);
}

void submitToQueue( VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore waitS, VkPipelineStageFlags waitStage, VkSemaphore signalS ){
	VkSubmitInfo submit{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr, // pNext
		1, // wait semaphore count
		&waitS,
		&waitStage, // pipeline stages to wait for semaphore
		1,
		&commandBuffer,
		1, // signal semaphore count
		&signalS // signal semaphores
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
