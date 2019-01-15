// Vulkan extensions commands loader

#ifndef EXTENSION_LOADER_H
#define EXTENSION_LOADER_H

#include <vector>

#include <unordered_map>

#include<cstring>

#include <vulkan/vulkan.h>

#include "CompilerMessages.h"
#include "EnumerateScheme.h"

void loadInstanceExtensionsCommands( VkInstance instance, const std::vector<const char*>& instanceExtensions );
void unloadInstanceExtensionsCommands( VkInstance instance );

void loadDeviceExtensionsCommands( VkDevice device, const std::vector<const char*>& instanceExtensions );
void unloadDeviceExtensionsCommands( VkDevice device );

void loadPDProps2Commands( VkInstance instance );
void unloadPDProps2Commands( VkInstance instance );

void loadDebugReportCommands( VkInstance instance );
void unloadDebugReportCommands( VkInstance instance );

void loadDebugUtilsCommands( VkInstance instance );
void unloadDebugUtilsCommands( VkInstance instance );

void loadExternalMemoryCapsCommands( VkInstance instance );
void unloadExternalMemoryCapsCommands( VkInstance instance );


void loadExternalMemoryCommands( VkDevice device );
void unloadExternalMemoryCommands( VkDevice device );

#ifdef VK_USE_PLATFORM_WIN32_KHR
void loadExternalMemoryWin32Commands( VkDevice device );
void unloadExternalMemoryWin32Commands( VkDevice device );
#endif

void loadDedicatedAllocationCommands( VkDevice device );
void unloadDedicatedAllocationCommands( VkDevice device );

////////////////////////////////////////////////////////

std::unordered_map< VkInstance, std::vector<const char*> > instanceExtensionsMap;
std::unordered_map< VkPhysicalDevice, VkInstance > physicalDeviceInstanceMap;

TODO( "Leaks destroyed instances" );
void populatePhysicalDeviceInstaceMap( const VkInstance instance ){
	const std::vector<VkPhysicalDevice> physicalDevices = enumerate<VkPhysicalDevice>( instance );
	for( const auto pd : physicalDevices ) physicalDeviceInstanceMap[pd] = instance;
}

void loadInstanceExtensionsCommands( const VkInstance instance, const std::vector<const char*>& instanceExtensions ){
	using std::strcmp;

	instanceExtensionsMap[instance] = instanceExtensions;

	for( const auto e : instanceExtensions ){
		if( strcmp( e, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME ) == 0 ) loadPDProps2Commands( instance );
		if( strcmp( e, VK_EXT_DEBUG_REPORT_EXTENSION_NAME ) == 0 ) loadDebugReportCommands( instance );
		if( strcmp( e, VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) == 0 ) loadDebugUtilsCommands( instance );
		if( strcmp( e, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME ) == 0 ) loadExternalMemoryCapsCommands( instance );
		// ...
	}
}

void unloadInstanceExtensionsCommands( const VkInstance instance ){
	using std::strcmp;

	for(  const auto e : instanceExtensionsMap.at( instance )  ){
		if( strcmp( e, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME ) == 0 ) unloadPDProps2Commands( instance );
		if( strcmp( e, VK_EXT_DEBUG_REPORT_EXTENSION_NAME ) == 0 ) unloadDebugReportCommands( instance );
		if( strcmp( e, VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) == 0 ) unloadDebugUtilsCommands( instance );
		if( strcmp( e, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME ) == 0 ) unloadExternalMemoryCapsCommands( instance );
		// ...
	}

	instanceExtensionsMap.erase( instance );
}

std::unordered_map< VkDevice, std::vector<const char*> > deviceExtensionsMap;

void loadDeviceExtensionsCommands( const VkDevice device, const std::vector<const char*>& deviceExtensions ){
	using std::strcmp;

	deviceExtensionsMap[device] = deviceExtensions;

	for( const auto e : deviceExtensions ){
		if( strcmp( e, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME ) == 0 ) loadExternalMemoryCommands( device );
#ifdef VK_USE_PLATFORM_WIN32_KHR
		if( strcmp( e, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME ) == 0 ) loadExternalMemoryWin32Commands( device );
#endif
		if( strcmp( e, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME ) == 0 ) loadDedicatedAllocationCommands( device );
		// ...
	}
}

void unloadDeviceExtensionsCommands( const VkDevice device ){
	using std::strcmp;

	for(  const auto e : deviceExtensionsMap.at( device )  ){
		if( strcmp( e, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME ) == 0 ) unloadExternalMemoryCommands( device );
#ifdef VK_USE_PLATFORM_WIN32_KHR
		if( strcmp( e, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME ) == 0 ) unloadExternalMemoryWin32Commands( device );
#endif
		if( strcmp( e, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME ) == 0 ) unloadDedicatedAllocationCommands( device );
		// ...
	}

	deviceExtensionsMap.erase( device );
}


// VK_KHR_get_physical_device_properties2
///////////////////////////////////////////

std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceFeatures2KHR > GetPhysicalDeviceFeatures2KHRDispatchTable;
std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceProperties2KHR > GetPhysicalDeviceProperties2KHRDispatchTable;
std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceFormatProperties2KHR > GetPhysicalDeviceFormatProperties2KHRDispatchTable;
std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceImageFormatProperties2KHR > GetPhysicalDeviceImageFormatProperties2KHRDispatchTable;
std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR > GetPhysicalDeviceQueueFamilyProperties2KHRDispatchTable;
std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceMemoryProperties2KHR > GetPhysicalDeviceMemoryProperties2KHRDispatchTable;
std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR > GetPhysicalDeviceSparseImageFormatProperties2KHRDispatchTable;

void loadPDProps2Commands( VkInstance instance ){
	populatePhysicalDeviceInstaceMap( instance );

	PFN_vkVoidFunction temp_fp;

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceFeatures2KHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceFeatures2KHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceFeatures2KHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceProperties2KHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceProperties2KHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceProperties2KHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceFormatProperties2KHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceFormatProperties2KHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceFormatProperties2KHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties2KHR>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceImageFormatProperties2KHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceImageFormatProperties2KHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceImageFormatProperties2KHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2KHR>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceQueueFamilyProperties2KHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceQueueFamilyProperties2KHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceMemoryProperties2KHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceMemoryProperties2KHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceMemoryProperties2KHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceSparseImageFormatProperties2KHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceSparseImageFormatProperties2KHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR>( temp_fp );
}

void unloadPDProps2Commands( VkInstance instance ){
	GetPhysicalDeviceFeatures2KHRDispatchTable.erase( instance );
	GetPhysicalDeviceProperties2KHRDispatchTable.erase( instance );
	GetPhysicalDeviceFormatProperties2KHRDispatchTable.erase( instance );
	GetPhysicalDeviceImageFormatProperties2KHRDispatchTable.erase( instance );
	GetPhysicalDeviceQueueFamilyProperties2KHRDispatchTable.erase( instance );
	GetPhysicalDeviceMemoryProperties2KHRDispatchTable.erase( instance );
	GetPhysicalDeviceSparseImageFormatProperties2KHRDispatchTable.erase( instance );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2KHR( VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures ){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceFeatures2KHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, pFeatures );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2KHR( VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties ){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceProperties2KHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, pProperties );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2KHR( VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties ){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceFormatProperties2KHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, format, pFormatProperties );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties2KHR( VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties ){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceImageFormatProperties2KHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, pImageFormatInfo, pImageFormatProperties );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2KHR( VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties ){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceQueueFamilyProperties2KHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2KHR( VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties ){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceMemoryProperties2KHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, pMemoryProperties );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2KHR( VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties ){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceSparseImageFormatProperties2KHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, pFormatInfo, pPropertyCount, pProperties );
}

// VK_EXT_debug_report
//////////////////////////////////

std::unordered_map< VkInstance, PFN_vkCreateDebugReportCallbackEXT > CreateDebugReportCallbackEXTDispatchTable;
std::unordered_map< VkInstance, PFN_vkDestroyDebugReportCallbackEXT > DestroyDebugReportCallbackEXTDispatchTable;
std::unordered_map< VkInstance, PFN_vkDebugReportMessageEXT > DebugReportMessageEXTDispatchTable;

void loadDebugReportCommands( VkInstance instance ){
	PFN_vkVoidFunction temp_fp;

	temp_fp = vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
	if( !temp_fp ) throw "Failed to load vkCreateDebugReportCallbackEXT"; // check shouldn't be necessary (based on spec)
	CreateDebugReportCallbackEXTDispatchTable[instance] = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
	if( !temp_fp ) throw "Failed to load vkDestroyDebugReportCallbackEXT"; // check shouldn't be necessary (based on spec)
	DestroyDebugReportCallbackEXTDispatchTable[instance] = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkDebugReportMessageEXT" );
	if( !temp_fp ) throw "Failed to load vkDebugReportMessageEXT"; // check shouldn't be necessary (based on spec)
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

// VK_EXT_debug_utils
//////////////////////////////////

std::unordered_map< VkInstance, PFN_vkCreateDebugUtilsMessengerEXT > CreateDebugUtilsMessengerEXTDispatchTable;
std::unordered_map< VkInstance, PFN_vkDestroyDebugUtilsMessengerEXT > DestroyDebugUtilsMessengerEXTDispatchTable;
std::unordered_map< VkInstance, PFN_vkSubmitDebugUtilsMessageEXT > SubmitDebugUtilsMessageEXTDispatchTable;

void loadDebugUtilsCommands( VkInstance instance ){
	PFN_vkVoidFunction temp_fp;

	temp_fp = vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
	if( !temp_fp ) throw "Failed to load vkCreateDebugUtilsMessengerEXT"; // check shouldn't be necessary (based on spec)
	CreateDebugUtilsMessengerEXTDispatchTable[instance] = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
	if( !temp_fp ) throw "Failed to load vkDestroyDebugUtilsMessengerEXT"; // check shouldn't be necessary (based on spec)
	DestroyDebugUtilsMessengerEXTDispatchTable[instance] = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( temp_fp );

	temp_fp = vkGetInstanceProcAddr( instance, "vkSubmitDebugUtilsMessageEXT" );
	if( !temp_fp ) throw "Failed to load vkSubmitDebugUtilsMessageEXT"; // check shouldn't be necessary (based on spec)
	SubmitDebugUtilsMessageEXTDispatchTable[instance] = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>( temp_fp );
}

void unloadDebugUtilsCommands( VkInstance instance ){
	CreateDebugUtilsMessengerEXTDispatchTable.erase( instance );
	DestroyDebugUtilsMessengerEXTDispatchTable.erase( instance );
	SubmitDebugUtilsMessageEXTDispatchTable.erase( instance );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pMessenger
){
	auto dispatched_cmd = CreateDebugUtilsMessengerEXTDispatchTable.at( instance );
	return dispatched_cmd( instance, pCreateInfo, pAllocator, pMessenger );
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT messenger,
	const VkAllocationCallbacks* pAllocator
){
	auto dispatched_cmd = DestroyDebugUtilsMessengerEXTDispatchTable.at( instance );
	return dispatched_cmd( instance, messenger, pAllocator );
}

VKAPI_ATTR void VKAPI_CALL vkSubmitDebugUtilsMessageEXT(
	VkInstance instance,
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
){
	auto dispatched_cmd = SubmitDebugUtilsMessageEXTDispatchTable.at( instance );
	return dispatched_cmd( instance, messageSeverity, messageTypes, pCallbackData );
}

// VK_KHR_external_memory_capabilities
///////////////////////////////////////////

std::unordered_map< VkInstance, PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR > GetPhysicalDeviceExternalBufferPropertiesKHRDispatchTable;

void loadExternalMemoryCapsCommands( VkInstance instance ){
	populatePhysicalDeviceInstaceMap( instance );

	PFN_vkVoidFunction temp_fp;

	temp_fp = vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR" );
	if( !temp_fp ) throw "Failed to load vkGetPhysicalDeviceExternalBufferPropertiesKHR"; // check shouldn't be necessary (based on spec)
	GetPhysicalDeviceExternalBufferPropertiesKHRDispatchTable[instance] = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR>( temp_fp );
}

void unloadExternalMemoryCapsCommands( VkInstance instance ){
	GetPhysicalDeviceExternalBufferPropertiesKHRDispatchTable.erase( instance );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalBufferPropertiesKHR(
	VkPhysicalDevice physicalDevice,
	const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
	VkExternalBufferProperties* pExternalBufferProperties
){
	const VkInstance instance = physicalDeviceInstanceMap.at( physicalDevice );
	auto dispatched_cmd = GetPhysicalDeviceExternalBufferPropertiesKHRDispatchTable.at( instance );
	return dispatched_cmd( physicalDevice, pExternalBufferInfo, pExternalBufferProperties );
}


// VK_KHR_external_memory
///////////////////////////////////////////

void loadExternalMemoryCommands( VkDevice ){
	// no commands
}

void unloadExternalMemoryCommands( VkDevice ){
	// no commands
}


#ifdef VK_USE_PLATFORM_WIN32_KHR
// VK_KHR_external_memory_win32
///////////////////////////////////////////

std::unordered_map< VkDevice, PFN_vkGetMemoryWin32HandleKHR > GetMemoryWin32HandleKHRDispatchTable;
std::unordered_map< VkDevice, PFN_vkGetMemoryWin32HandlePropertiesKHR > GetMemoryWin32HandlePropertiesKHRDispatchTable;

void loadExternalMemoryWin32Commands( VkDevice device ){
	PFN_vkVoidFunction temp_fp;

	temp_fp = vkGetDeviceProcAddr( device, "vkGetMemoryWin32HandleKHR" );
	if( !temp_fp ) throw "Failed to load vkGetMemoryWin32HandleKHR"; // check shouldn't be necessary (based on spec)
	GetMemoryWin32HandleKHRDispatchTable[device] = reinterpret_cast<PFN_vkGetMemoryWin32HandleKHR>( temp_fp );

	temp_fp = vkGetDeviceProcAddr( device, "vkGetMemoryWin32HandlePropertiesKHR" );
	if( !temp_fp ) throw "Failed to load vkGetMemoryWin32HandlePropertiesKHR"; // check shouldn't be necessary (based on spec)
	GetMemoryWin32HandlePropertiesKHRDispatchTable[device] = reinterpret_cast<PFN_vkGetMemoryWin32HandlePropertiesKHR>( temp_fp );
}

void unloadExternalMemoryWin32Commands( VkDevice device ){
	GetMemoryWin32HandleKHRDispatchTable.erase( device );
	GetMemoryWin32HandlePropertiesKHRDispatchTable.erase( device );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryWin32HandleKHR(
	VkDevice device,
	const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
	HANDLE* pHandle
){
	auto dispatched_cmd = GetMemoryWin32HandleKHRDispatchTable.at( device );
	return dispatched_cmd( device, pGetWin32HandleInfo, pHandle );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryWin32HandlePropertiesKHR(
	VkDevice device,
	VkExternalMemoryHandleTypeFlagBits handleType,
	HANDLE handle,
	VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties
){
	auto dispatched_cmd = GetMemoryWin32HandlePropertiesKHRDispatchTable.at( device );
	return dispatched_cmd( device, handleType, handle, pMemoryWin32HandleProperties );
}
#endif

// VK_KHR_dedicated_allocation
///////////////////////////////////////////

void loadDedicatedAllocationCommands( VkDevice ){
	// no commands
}

void unloadDedicatedAllocationCommands( VkDevice ){
	// no commands
}

#endif //EXTENSION_LOADER_H