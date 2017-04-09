// Vulkan extensions commands loader

#ifndef EXTENSION_LOADER_H
#define EXTENSION_LOADER_H

#include <vector>
using std::vector;

#include <unordered_map>
using std::unordered_map;

#include<cstring>

#include <vulkan/vulkan.h>

void loadInstanceExtensionsCommands( VkInstance instance, vector<const char*> instanceExtensions );
void unloadInstanceExtensionsCommands( VkInstance instance );

void loadDeviceExtensionsCommands( VkDevice device, vector<const char*> instanceExtensions );
void unloadDeviceExtensionsCommands( VkDevice device );

void loadDebugReportCommands( VkInstance instance );
void unloadDebugReportCommands( VkInstance instance );


////////////////////////////////////////////////////////

unordered_map< VkInstance, vector<const char*> > instanceExtensionsMap;

void loadInstanceExtensionsCommands( VkInstance instance, vector<const char*> instanceExtensions ){
	using std::strcmp;

	instanceExtensionsMap[instance] = instanceExtensions;

	for( const auto e : instanceExtensions ){
		if( strcmp( e, VK_EXT_DEBUG_REPORT_EXTENSION_NAME ) == 0 ) loadDebugReportCommands( instance );
		// ...
	}
}

void unloadInstanceExtensionsCommands( VkInstance instance ){
	using std::strcmp;

	for(  const auto e : instanceExtensionsMap.at( instance )  ){
		if( strcmp( e, VK_EXT_DEBUG_REPORT_EXTENSION_NAME ) == 0 ) unloadDebugReportCommands( instance );
		// ...
	}

	instanceExtensionsMap.erase( instance );
}

unordered_map< VkDevice, vector<const char*> > deviceExtensionsMap;

void loadDeviceExtensionsCommands( VkDevice device, vector<const char*> deviceExtensions ){
	using std::strcmp;

	deviceExtensionsMap[device] = deviceExtensions;

	//for( const auto e : deviceExtensions ){
	//	// ...
	//}
}

void unloadDeviceExtensionsCommands( VkDevice device ){
	using std::strcmp;

	//for(  const auto e : deviceExtensionsMap.at( device )  ){
	//	// ...
	//}

	deviceExtensionsMap.erase( device );
}

// VK_EXT_debug_report
//////////////////////////////////

unordered_map< VkInstance, PFN_vkCreateDebugReportCallbackEXT > CreateDebugReportCallbackEXTDispatchTable;
unordered_map< VkInstance, PFN_vkDestroyDebugReportCallbackEXT > DestroyDebugReportCallbackEXTDispatchTable;
unordered_map< VkInstance, PFN_vkDebugReportMessageEXT > DebugReportMessageEXTDispatchTable;

void loadDebugReportCommands( VkInstance instance ){
	PFN_vkVoidFunction temp_fp;

	temp_fp = vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
	if( !temp_fp ) throw "Failed to load vkCreateDebugReportCallbackEXT";
	CreateDebugReportCallbackEXTDispatchTable[instance] = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
	if( !temp_fp ) throw "Failed to load vkDestroyDebugReportCallbackEXT";
	DestroyDebugReportCallbackEXTDispatchTable[instance] = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkDebugReportMessageEXT" );
	if( !temp_fp ) throw "Failed to load vkDebugReportMessageEXT";
	DebugReportMessageEXTDispatchTable[instance] = reinterpret_cast<PFN_vkDebugReportMessageEXT>( temp_fp );
}

void unloadDebugReportCommands( VkInstance instance ){
	CreateDebugReportCallbackEXTDispatchTable.erase( instance );
	DestroyDebugReportCallbackEXTDispatchTable.erase( instance );
	DebugReportMessageEXTDispatchTable.erase( instance );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback
){
	auto dispatched_cmd = CreateDebugReportCallbackEXTDispatchTable.at( instance );
	return dispatched_cmd( instance, pCreateInfo, pAllocator, pCallback );
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks* pAllocator
){
	auto dispatched_cmd = DestroyDebugReportCallbackEXTDispatchTable.at( instance );
	return dispatched_cmd( instance, callback, pAllocator );
}

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
	auto dispatched_cmd = DebugReportMessageEXTDispatchTable.at( instance );
	return dispatched_cmd( instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage );
}


#endif //EXTENSION_LOADER_H