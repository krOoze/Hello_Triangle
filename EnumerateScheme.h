// Scheme for the various vkGet* and vkEnumerate* commands
//
// Following the DRY principle, this implements getting a vector of enumerants
// from the similar enumeration commands that return VK_INCOMPELTE.

#ifndef ENUMERATE_SCHEME_H
#define ENUMERATE_SCHEME_H

#include <vector>
#include <functional>

#include <vulkan/vulkan.h>

#include "ErrorHandling.h"

// the enumeration scheme
// takes function VkResult cmd( uint32_t count, Element* pArray ) and Vulkan command name (for debugging purposes)
// returns vector<Element> which contains the enumerants, or throws
template< typename Element, typename Cmd >
decltype(auto) enumerateScheme( Cmd cmd, const char* cmdName ){
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

template< typename Element >
std::vector<Element> enumerate();

template< typename Element, typename Source >
std::vector<Element> enumerate( Source );

template< typename Element, typename Dispatch >
std::vector<Element> enumerate( Dispatch );

template< typename Element, typename Dispatch, typename Source >
std::vector<Element> enumerate( Dispatch, Source );

// for vkEnumerateInstanceLayerProperties -- auto v = enumerate<VkLayerProperties>();
TODO( "Considering adding tag types here for clarity; e.g. enumerate<VkInstance, VkLayerProperties>() and enumerate<VkDevice, VkLayerProperties>()." )
template<>
std::vector<VkLayerProperties> enumerate<VkLayerProperties>(){
	return enumerateScheme<VkLayerProperties>( vkEnumerateInstanceLayerProperties, "vkEnumerateInstanceLayerProperties" );
}

// for vkEnumerateDeviceLayerProperties -- auto v = enumerate<VkLayerProperties>( pd );
template<>
std::vector<VkLayerProperties> enumerate<VkLayerProperties>( VkPhysicalDevice physicalDevice ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkEnumerateDeviceLayerProperties, physicalDevice, _1, _2 );

	return enumerateScheme<VkLayerProperties>( adapterCmd, "vkEnumerateDeviceLayerProperties" );
}

// for vkEnumerateInstanceExtensionProperties -- auto v = enumerate<VkExtensionProperties>( "ln" );
template<>
std::vector<VkExtensionProperties> enumerate<VkExtensionProperties>( const char* pLayerName ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkEnumerateInstanceExtensionProperties, pLayerName, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateInstanceExtensionProperties" );
}

// for vkEnumerateInstanceExtensionProperties with nullptr layer -- auto v = enumerate<VkExtensionProperties>();
template<>
std::vector<VkExtensionProperties> enumerate<VkExtensionProperties>(){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkEnumerateInstanceExtensionProperties, nullptr, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateInstanceExtensionProperties" );
}

// for vkEnumerateDeviceExtensionProperties -- auto v = enumerate<VkExtensionProperties>( pd, "ln" );
template<>
std::vector<VkExtensionProperties> enumerate<VkExtensionProperties>( VkPhysicalDevice physicalDevice, const char* pLayerName ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkEnumerateDeviceExtensionProperties, physicalDevice, pLayerName, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateDeviceExtensionProperties" );
}

// for vkEnumerateInstanceExtensionProperties with nullptr layer -- auto v = enumerate<VkExtensionProperties>( pd );
template<>
std::vector<VkExtensionProperties> enumerate<VkExtensionProperties>( VkPhysicalDevice physicalDevice ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkEnumerateDeviceExtensionProperties, physicalDevice, nullptr, _1, _2 );

	return enumerateScheme<VkExtensionProperties>( adapterCmd, "vkEnumerateDeviceExtensionProperties" );
}

// for vkEnumeratePhysicalDevices -- auto v = enumerate<VkPhysicalDevice>( i );
template<>
std::vector<VkPhysicalDevice> enumerate<VkPhysicalDevice>( VkInstance instance ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkEnumeratePhysicalDevices, instance, _1, _2 );

	return enumerateScheme<VkPhysicalDevice>( adapterCmd, "vkEnumeratePhysicalDevices" );
}

// for vkGetPhysicalDeviceSurfaceFormatsKHR -- auto v = enumerate<VkSurfaceFormatKHR>( pd, s );
template<>
std::vector<VkSurfaceFormatKHR> enumerate<VkSurfaceFormatKHR>( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkGetPhysicalDeviceSurfaceFormatsKHR, physicalDevice, surface, _1, _2 );

	return enumerateScheme<VkSurfaceFormatKHR>( adapterCmd, "vkGetPhysicalDeviceSurfaceFormatsKHR" );
}

// for vkGetPhysicalDeviceSurfacePresentModesKHR -- auto v = enumerate<VkSurfaceFormatKHR>( pd, s );
template<>
std::vector<VkPresentModeKHR> enumerate<VkPresentModeKHR>( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkGetPhysicalDeviceSurfacePresentModesKHR, physicalDevice, surface, _1, _2 );

	return enumerateScheme<VkPresentModeKHR>( adapterCmd, "vkGetPhysicalDeviceSurfacePresentModesKHR" );
}

// for vkGetSwapchainImagesKHR -- auto v = enumerate<VkSurfaceFormatKHR>( d, s );
template<>
std::vector<VkImage> enumerate<VkImage>( VkDevice device, VkSwapchainKHR swapchain ){
	using namespace std::placeholders;
	const auto adapterCmd = std::bind( vkGetSwapchainImagesKHR, device, swapchain, _1, _2 );

	return enumerateScheme<VkImage>( adapterCmd, "vkGetSwapchainImagesKHR" );
}

// ... others to be added as needed

#endif //ENUMERATE_SCHEME_H