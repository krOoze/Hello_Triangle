// Scheme for the various vkGet* and vkEnumerate* commands
//
// Following the DRY principle, this implements getting a vector of enumerants
// from the similar enumeration commands that return VK_INCOMPELTE.

#ifndef COMMON_ENUMERATE_SCHEME_H
#define COMMON_ENUMERATE_SCHEME_H

#include <functional>
#include <type_traits>
#include <vector>

#include <vulkan/vulkan.h>

#include "CompilerMessages.h"
#include "ErrorHandling.h"

// the enumeration scheme
// takes function VkResult cmd( uint32_t count, Element* pArray ) and Vulkan command name (for debugging purposes)
// returns vector<Element> which contains the enumerants, or throws
template< typename Element, typename Cmd >
std::vector<Element> enumerateScheme( Cmd cmd, const char* cmdName ){
	std::vector<Element> enumerants;

	VkResult errorCode;
	uint32_t enumerantsCount;

	// repeat until complete array is returned, or error
	do{
		errorCode = cmd( &enumerantsCount, nullptr ); RESULT_HANDLER( errorCode, cmdName ); // get current array size

		enumerants.resize( enumerantsCount );
		errorCode = cmd( &enumerantsCount, enumerants.data() ); // get current array up to enumerantsCount
	} while( errorCode == VK_INCOMPLETE );

	RESULT_HANDLER( errorCode, cmdName );

	enumerants.resize( enumerantsCount ); // shrink in case of enumerantsCount1 > enumerantsCount2
	enumerants.shrink_to_fit(); // unlikely the vector will grow from this point on anyway

	return enumerants;
}


// Adapters for specific Vulkan commands
///////////////////////////////////////////////

template< typename Element, typename... Ts, typename = std::enable_if_t<!std::is_same<Element, VkInstance>::value>  >
std::vector<Element> enumerate( Ts... );

// Tag will be VkInstance if to disambiguate commands that also work on device
template< typename Tag, typename Element, typename... Ts, typename = std::enable_if_t<std::is_same<Tag, VkInstance>::value> >
std::vector<Element> enumerate( Ts... );

// for vkEnumerateInstanceLayerProperties -- auto v = enumerate<VkInstance, VkLayerProperties>();
template<>
std::vector<VkLayerProperties> enumerate<VkInstance, VkLayerProperties>(){
	return enumerateScheme<VkLayerProperties>( vkEnumerateInstanceLayerProperties, "vkEnumerateInstanceLayerProperties" );
}

// for vkEnumerateDeviceLayerProperties -- auto v = enumerate<VkLayerProperties>( pd );
template<>
std::vector<VkLayerProperties> enumerate<VkLayerProperties, VkPhysicalDevice>( VkPhysicalDevice physicalDevice ){
	using namespace std::placeholders;
	const auto cmd = vkEnumerateDeviceLayerProperties;
	const auto adapterCmd = std::bind( cmd, physicalDevice, _1, _2 );

	return enumerateScheme<VkLayerProperties>( adapterCmd, "vkEnumerateDeviceLayerProperties" );
}

// for vkEnumerateInstanceExtensionProperties -- auto v = enumerate<VkInstance, VkExtensionProperties>( "ln" );
template<>
std::vector<VkExtensionProperties> enumerate<VkInstance, VkExtensionProperties, const char*>( const char* pLayerName ){
	using namespace std::placeholders;
	const auto cmd = vkEnumerateInstanceExtensionProperties;
	const auto adapterCmd = std::bind( cmd, pLayerName, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateInstanceExtensionProperties" );
}

// for vkEnumerateInstanceExtensionProperties with nullptr layer -- auto v = enumerate<VkInstance, VkExtensionProperties>();
template<>
std::vector<VkExtensionProperties> enumerate<VkInstance, VkExtensionProperties>(){
	using namespace std::placeholders;
	const auto cmd = vkEnumerateInstanceExtensionProperties;
	const auto adapterCmd = std::bind( cmd, nullptr, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateInstanceExtensionProperties" );
}

// for vkEnumerateDeviceExtensionProperties -- auto v = enumerate<VkExtensionProperties>( pd, "ln" );
template<>
std::vector<VkExtensionProperties> enumerate<VkExtensionProperties, VkPhysicalDevice, const char*>( VkPhysicalDevice physicalDevice, const char* pLayerName ){
	using namespace std::placeholders;
	const auto cmd = vkEnumerateDeviceExtensionProperties;
	const auto adapterCmd = std::bind( cmd, physicalDevice, pLayerName, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateDeviceExtensionProperties" );
}

// for vkEnumerateInstanceExtensionProperties with nullptr layer -- auto v = enumerate<VkExtensionProperties>( pd );
template<>
std::vector<VkExtensionProperties> enumerate<VkExtensionProperties, VkPhysicalDevice>( VkPhysicalDevice physicalDevice ){
	using namespace std::placeholders;
	const auto cmd = vkEnumerateDeviceExtensionProperties;
	const auto adapterCmd = std::bind( cmd, physicalDevice, nullptr, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateDeviceExtensionProperties" );
}

// for vkEnumeratePhysicalDevices -- auto v = enumerate<VkPhysicalDevice>( i );
template<>
std::vector<VkPhysicalDevice> enumerate<VkPhysicalDevice, VkInstance>( VkInstance instance ){
	using namespace std::placeholders;
	const auto cmd = vkEnumeratePhysicalDevices;
	const auto adapterCmd = std::bind( cmd, instance, _1, _2 );

	return enumerateScheme<VkPhysicalDevice>( adapterCmd, "vkEnumeratePhysicalDevices" );
}

// for vkGetPhysicalDeviceSurfaceFormatsKHR -- auto v = enumerate<VkSurfaceFormatKHR>( pd, s );
template<>
std::vector<VkSurfaceFormatKHR> enumerate<VkSurfaceFormatKHR, VkPhysicalDevice, VkSurfaceKHR>( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	using namespace std::placeholders;
	const auto cmd = vkGetPhysicalDeviceSurfaceFormatsKHR;
	const auto adapterCmd = std::bind( cmd, physicalDevice, surface, _1, _2 );

	return enumerateScheme<VkSurfaceFormatKHR>( adapterCmd, "vkGetPhysicalDeviceSurfaceFormatsKHR" );
}

// for vkGetPhysicalDeviceSurfacePresentModesKHR -- auto v = enumerate<VkSurfaceFormatKHR>( pd, s );
template<>
std::vector<VkPresentModeKHR> enumerate<VkPresentModeKHR, VkPhysicalDevice, VkSurfaceKHR>( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	using namespace std::placeholders;
	const auto cmd = vkGetPhysicalDeviceSurfacePresentModesKHR;
	const auto adapterCmd = std::bind( cmd, physicalDevice, surface, _1, _2 );

	return enumerateScheme<VkPresentModeKHR>( adapterCmd, "vkGetPhysicalDeviceSurfacePresentModesKHR" );
}

// for vkGetSwapchainImagesKHR -- auto v = enumerate<VkSurfaceFormatKHR>( d, s );
template<>
std::vector<VkImage> enumerate<VkImage, VkDevice, VkSwapchainKHR>( VkDevice device, VkSwapchainKHR swapchain ){
	using namespace std::placeholders;
	const auto cmd = vkGetSwapchainImagesKHR;
	const auto adapterCmd = std::bind( cmd, device, swapchain, _1, _2 );

	return enumerateScheme<VkImage>( adapterCmd, "vkGetSwapchainImagesKHR" );
}

// ... others to be added as needed

#endif //COMMON_ENUMERATE_SCHEME_H