// Template helper for many of the vkGet* and vkEnumerate* commands
//
// They are all used almost the same,
// so this template's purpose is to help DRY principle.

#ifndef ENUMERATE_SCHEME_H
#define ENUMERATE_SCHEME_H

#include <vector>
#include <functional>
#include <tuple>
#include <type_traits>

#include <vulkan/vulkan.h>

#include "FunctionTraits.h"


// scheme for all Vulkan style enumerations
// count + array of Elements are the common parameters for all the kinds of Vulkan enumeration comands
//
// it can be used directly with command taking two parameters [count, out array]
// e.g. for vkEnumerateInstanceLayerProperties( uint32_t*, VkLayerProperties* )
// vkEnumerateInstanceLayerProperties being the Cmd cmd
// usage: vector<VkLayerProperties> lps = enumerate( vkEnumerateInstanceLayerProperties, "vkEnumerateInstanceLayerProperties" );
template< typename Cmd >
decltype(auto) enumerate( Cmd cmd, const char* cmdName ){
	using Element = std::remove_pointer_t<  function_param_t< Cmd, 1 >  >; // i.e. the 2nd parameter type of the cmd (dereferenced)

	vector<Element> enumerants;

	VkResult errorCode;
	uint32_t enumerantsCount;

	// repeat until complete array is returned, or error
	do{
		errorCode = cmd( &enumerantsCount, nullptr ); RESULT_HANDLER( errorCode, cmdName ); // get current array size

		enumerants.resize( enumerantsCount );
		errorCode = cmd( &enumerantsCount, enumerants.data() ); // get current array up to enumerantsCount
	} while (errorCode == VK_INCOMPLETE);

	RESULT_HANDLER( errorCode, cmdName );

	enumerants.resize( enumerantsCount ); // shrink in case of enumerantsCount1 > enumerantsCount2
	enumerants.shrink_to_fit(); // unlikely the vector will grow from this point on anyway

	return enumerants;
}


// scheme adaptor for 3 parameter Vulkan enumerations [dispatch, count, out array]
// or for global Vulkan enumerations [source, count, out array]
//
// e.g. for vkEnumeratePhysicalDevices( VkInstance, uint32_t*, VkPhysicalDevice* );
// vkEnumeratePhysicalDevices being the Cmd cmd, and the instance being the Dispatch d
// usage: vector<VkPhysicalDevice> pds = enumerate( vkEnumeratePhysicalDevices, instance, "vkEnumeratePhysicalDevices" );
// or
// e.g. for vkEnumerateInstanceExtensionProperties( const char*, uint32_t*, VkExtensionProperties* );
// vkEnumerateInstanceExtensionProperties being the Cmd cmd, and the layer name being the Source s
// usage: vector<VkExtensionProperties> eps = enumerate( vkEnumerateInstanceExtensionProperties, nullptr, "vkEnumerateInstanceExtensionProperties" );
template<
	typename Cmd,
	typename DispatchOrSource
>
decltype(auto) enumerate( Cmd cmd, DispatchOrSource dors, const char* cmdName ){
	using ElementArray = function_param_t< Cmd, 2 >; // i.e. the 3rd parameter type of the cmd

	std::function<  VkResult( uint32_t*, ElementArray )  > adapted_cmd = std::bind(
		cmd,
		dors,
		std::placeholders::_1,
		std::placeholders::_2
	);

	return enumerate( adapted_cmd, cmdName );
}


// scheme adaptor for 4 parameter Vulkan enumerations [dispatch, source, count, out array]
//
// e.g. for vkGetPhysicalDeviceSurfaceFormatsKHR( VkPhysicalDevice, VkSurfaceKHR, uint32_t*, VkSurfaceFormatKHR* );
// vkGetPhysicalDeviceSurfaceFormatsKHR being the Cmd cmd, the physdevice being the Dispatch d, and the surface being the Source s
// usage: vector<VkSurfaceFormatKHR> sfs = enumerate( vkGetPhysicalDeviceSurfaceFormatsKHR, instance, surface, "vkGetPhysicalDeviceSurfaceFormatsKHR" );
template<
	typename Cmd,
	typename Dispatch,
	typename Source
>
decltype(auto) enumerate( Cmd cmd, Dispatch d, Source s, const char* cmdName ){
	using ElementArray = function_param_t< Cmd, 3 >; // i.e. the 4th parameter type of the cmd

	std::function<  VkResult( uint32_t*, ElementArray )  > adapted_cmd = std::bind(
		cmd,
		d,
		s,
		std::placeholders::_1,
		std::placeholders::_2
	);

	return enumerate( adapted_cmd, cmdName );
}


#endif //ENUMERATE_SCHEME_H