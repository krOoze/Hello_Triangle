// Introspection for Vulkan enums -- mostly to_string

#ifndef COMMON_VULKAN_INTROSPECTION_H
#define COMMON_VULKAN_INTROSPECTION_H

#include <string>
#include <sstream>

#include <vulkan/vulkan.h>


template <typename PHANDLE_T>
inline uint64_t handleToUint64(const PHANDLE_T *h) { return reinterpret_cast<uint64_t>(h); }
inline uint64_t handleToUint64(const uint64_t h) { return h; }

const char* to_string( const VkResult r ){
	switch( r ){
		case VK_SUCCESS:                                            return "VK_SUCCESS";
		case VK_NOT_READY:                                          return "VK_NOT_READY";
		case VK_TIMEOUT:                                            return "VK_TIMEOUT";
		case VK_EVENT_SET:                                          return "VK_EVENT_SET";
		case VK_EVENT_RESET:                                        return "VK_EVENT_RESET";
		case VK_INCOMPLETE:                                         return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:                           return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:                        return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:                                  return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:                            return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:                            return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:                        return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:                          return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:                          return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:                             return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:                              return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:                                      return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY:                           return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION:                                return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:               return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_ERROR_SURFACE_LOST_KHR:                             return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:                                     return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:                              return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:                        return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV:                            return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_NOT_PERMITTED_EXT:                            return "VK_ERROR_NOT_PERMITTED_EXT";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR:                                    return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR:                                    return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR:                             return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:                         return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_PIPELINE_COMPILE_REQUIRED_EXT:                      return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		default:                                                    return "unrecognized VkResult code";
	}
}

std::string to_string( const VkDebugReportObjectTypeEXT o ){
	switch( o ){
		case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT:                    return "unknown";
		case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:                   return "Instance";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT:            return "PhysicalDevice";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:                     return "Device";
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:                      return "Queue";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:                  return "Semaphore";
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:             return "CommandBuffer";
		case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:                      return "Fence";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:              return "DeviceMemory";
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:                     return "Buffer";
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:                      return "Image";
		case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:                      return "Event";
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:                 return "QueryPool";
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:                return "BufferView";
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:                 return "ImageView";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:              return "ShaderModule";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:             return "PipelineCache";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT:            return "PipelineLayout";
		case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:                return "RenderPass";
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:                   return "Pipeline";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT:      return "DescriptorSetLayout";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:                    return "Sampler";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT:            return "DescriptorPool";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:             return "DescriptorSet";
		case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:                return "Framebuffer";
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:               return "Command pool";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:                return "SurfaceKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:              return "SwapchainKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT:  return "DebugReportCallbackEXT";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT:                return "DisplayKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT:           return "DisplayModeKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT:       return "ValidationCacheEXT";
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:   return "SamplerYcbcrConversion";
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT: return "DescriptorUpdateTemplate";
		case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT: return "AccelerationStructureKHR";
		case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT:  return "AccelerationStructureNV";
		default:                                                         return "unrecognized type";
	}
}

std::string to_string( const VkObjectType o ){
	switch( o ){
		case VK_OBJECT_TYPE_UNKNOWN:                         return "unknown";
		case VK_OBJECT_TYPE_INSTANCE:                        return "Instance";
		case VK_OBJECT_TYPE_PHYSICAL_DEVICE:                 return "PhysicalDevice";
		case VK_OBJECT_TYPE_DEVICE:                          return "Device";
		case VK_OBJECT_TYPE_QUEUE:                           return "Queue";
		case VK_OBJECT_TYPE_SEMAPHORE:                       return "Semaphore";
		case VK_OBJECT_TYPE_COMMAND_BUFFER:                  return "CommandBuffer";
		case VK_OBJECT_TYPE_FENCE:                           return "Fence";
		case VK_OBJECT_TYPE_DEVICE_MEMORY:                   return "DeviceMemory";
		case VK_OBJECT_TYPE_BUFFER:                          return "Buffer";
		case VK_OBJECT_TYPE_IMAGE:                           return "Image";
		case VK_OBJECT_TYPE_EVENT:                           return "Event";
		case VK_OBJECT_TYPE_QUERY_POOL:                      return "QueryPool";
		case VK_OBJECT_TYPE_BUFFER_VIEW:                     return "BufferView";
		case VK_OBJECT_TYPE_IMAGE_VIEW:                      return "ImageView";
		case VK_OBJECT_TYPE_SHADER_MODULE:                   return "ShaderModule";
		case VK_OBJECT_TYPE_PIPELINE_CACHE:                  return "PipelineCache";
		case VK_OBJECT_TYPE_PIPELINE_LAYOUT:                 return "PipelineLayout";
		case VK_OBJECT_TYPE_RENDER_PASS:                     return "RenderPass";
		case VK_OBJECT_TYPE_PIPELINE:                        return "Pipeline";
		case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:           return "DescriptorSetLayout";
		case VK_OBJECT_TYPE_SAMPLER:                         return "Sampler";
		case VK_OBJECT_TYPE_DESCRIPTOR_POOL:                 return "DescriptorPool";
		case VK_OBJECT_TYPE_DESCRIPTOR_SET:                  return "DescriptorSet";
		case VK_OBJECT_TYPE_FRAMEBUFFER:                     return "Framebuffer";
		case VK_OBJECT_TYPE_COMMAND_POOL:                    return "Command pool";
		case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:        return "SamplerYcbcrConversion";
		case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:      return "DescriptorUpdateTemplateKHR";
		case VK_OBJECT_TYPE_SURFACE_KHR:                     return "SurfaceKHR";
		case VK_OBJECT_TYPE_SWAPCHAIN_KHR:                   return "SwapchainKHR";
		case VK_OBJECT_TYPE_DISPLAY_KHR:                     return "DisplayKHR";
		case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:                return "DisplayModeKHR";
		case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:       return "DebugReportCallbackEXT";
#ifdef VK_ENABLE_BETA_EXTENSIONS
		case VK_OBJECT_TYPE_VIDEO_SESSION_KHR: return "VideoSessionKHR";
		case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR: return "VideoSessionParametersKHR";
#endif
		case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:       return "DebugUtilsMessengerEXT";
		case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:      return "AccelerationStructureKHR";
		case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:            return "ValidationCacheEXT";
		case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:       return "AccelerationStructureNV";
		case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL: return "PerformanceConfigurationINTEL";
		case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:          return "DeferredOperationKHR";
		case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:     return "IndirectCommandsLayoutNV";
		case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT:           return "PrivateDataSlotEXT";
		default:                                             return "unrecognized type";
	}
}

std::string dbrflags_to_string( VkDebugReportFlagsEXT msgFlags ){
	std::string res;
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

std::string dbuseverity_to_string( const VkDebugUtilsMessageSeverityFlagsEXT debugSeverity ){
	std::string res;
	bool first = true;

	if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ){
		if( !first ) res += " | ";
		res += "ERROR";
		first = false;
	}

	if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ){
		if( !first ) res += " | ";
		res += "WARNING";
		first = false;
	}

	if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ){
		if( !first ) res += " | ";
		res += "Info";
		first = false;
	}

	if( debugSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ){
		if( !first ) res += " | ";
		res += "Verbose";
		first = false;
	}

	VkDebugUtilsMessageSeverityFlagsEXT known = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	if( debugSeverity & ~known ){
		if( !first ) res += " | ";
		res += "UNRECOGNIZED_FLAG";
		first = false;
	}

	return res;
}

std::string to_string( const VkDebugUtilsMessageSeverityFlagBitsEXT debugSeverity ){
	return dbuseverity_to_string( debugSeverity );
}

std::string dbutype_to_string( const VkDebugUtilsMessageTypeFlagsEXT debugType ){
	std::string res;
	bool first = true;

	if( debugType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ){
		if( !first ) res += " | ";
		res += "GENERAL";
		first = false;
	}

	if( debugType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ){
		if( !first ) res += " | ";
		res += "VALIDATION";
		first = false;
	}

	if( debugType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ){
		if( !first ) res += " | ";
		res += "PERFORMANCE";
		first = false;
	}

	VkDebugUtilsMessageSeverityFlagsEXT known = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	if( debugType & ~known ){
		if( !first ) res += " | ";
		res += "UNRECOGNIZED_FLAG";
		first = false;
	}

	return res;
}

std::string to_string_hex( const uint64_t n ){
	std::stringstream ss;
	ss << std::hex << std::noshowbase << std::uppercase << n;
	return "0x" + ss.str();
}

#endif //COMMON_VULKAN_INTROSPECTION_H