// Reusable error handling primitives for Vulkan

#ifndef COMMON_ERROR_HANDLING_H
#define COMMON_ERROR_HANDLING_H

#include <iostream>
#include <string>
#include <sstream>

#include <vulkan/vulkan.h>

#include "CompilerMessages.h"

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

// just use cout for logging now
std::ostream& logger = std::cout;

VKAPI_ATTR VkBool32 VKAPI_CALL genericDebugCallback(
	VkDebugReportFlagsEXT msgFlags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t /*location*/,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* /*pUserData*/
);

VkDebugReportCallbackEXT initDebug( VkInstance instance, const VkDebugReportFlagsEXT debugAmount );
void killDebug( VkInstance instance, VkDebugReportCallbackEXT debug );


template <typename PHANDLE_T>
inline uint64_t handleToUint64(const PHANDLE_T *h) { return reinterpret_cast<uint64_t>(h); }
inline uint64_t handleToUint64(const uint64_t h) { return h; }

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
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT: return "DebugReportCallbackEXT";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT: return "DisplayKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT: return "DisplayModeKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT: return "ObjectTableNVX";
		case VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT: return "IndirectCommandsLayoutNVX";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT: return "DescriptorUpdateTemplateKHR";
		default: return "unrecognized type";
	}
}

string d_to_string( VkDebugReportFlagsEXT msgFlags ){
	string res;
	bool first = true;

	if( msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT ){
		if( !first ) res += " | ";
		res += "ERROR";
		first = false;
	}

	if( msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT ){
		if( !first ) res += " | ";
		res += "WARNING";
		first = false;
	}

	if( msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT ){
		if( !first ) res += " | ";
		res += "PERFORMANCE";
		first = false;
	}

	if( msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT ){
		if( !first ) res += " | ";
		res += "Info";
		first = false;
	}

	if( msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT ){
		if( !first ) res += " | ";
		res += "Debug";
		first = false;
	}

	VkDebugReportFlagsEXT known = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	if( msgFlags & ~known ){
		if( !first ) res += " | ";
		res += "UNRECOGNIZED_FLAG";
		first = false;
	}

	return res;
}

string to_string_hex( const uint64_t n ){
	std::stringstream ss;
	ss << std::hex << std::noshowbase << std::uppercase << n;
	return "0x" + ss.str();
}

VKAPI_ATTR VkBool32 VKAPI_CALL genericDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t /*location*/,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* /*pUserData*/
){
	using std::endl;
	using std::to_string;

	const std::string report = d_to_string( flags ) + ": " + to_string( objectType ) + "(" + to_string_hex( object ) + ")" + ": " + to_string( messageCode ) + ", " + pLayerPrefix + ", " + pMessage;

	if( (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) || (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) || (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) ){
			logger << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			logger << report;
			logger << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			logger << endl;
	}
	else{
		logger << report << endl;
	}

	return VK_FALSE; // no abort on misbehaving command
}


VkDebugReportCallbackEXT initDebug( const VkInstance instance, const VkDebugReportFlagsEXT debugAmount ){
	const VkDebugReportCallbackCreateInfoEXT debugCreateInfo{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
		nullptr, // pNext
		debugAmount,
		::genericDebugCallback,
		nullptr // pUserData
	};

	VkDebugReportCallbackEXT debug;
	const VkResult errorCode = vkCreateDebugReportCallbackEXT( instance, &debugCreateInfo, nullptr, &debug ); RESULT_HANDLER( errorCode, "vkCreateDebugReportCallbackEXT" );

	return debug;
}

void killDebug( const VkInstance instance, const VkDebugReportCallbackEXT debug ){
	vkDestroyDebugReportCallbackEXT( instance, debug, nullptr );
}

#endif //COMMON_ERROR_HANDLING_H
