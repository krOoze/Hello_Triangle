// vulkan.h header global settings

// Should be included as soon as possible (first line in translation unit preferably)

#ifndef COMMON_VULKAN_ENVIRONMENT_H
#define COMMON_VULKAN_ENVIRONMENT_H

// detect premature vulkan.h
#ifdef VULKAN_H_
	#error "vulkan.h is included before VulkanEnvironment.h!"
#endif //VULKAN_H_

#include "CompilerMessages.h"


#ifdef NDEBUG
	#ifndef VULKAN_VALIDATION
		#define VULKAN_VALIDATION 0
	#endif
#else
	#ifndef VULKAN_VALIDATION
		#define VULKAN_VALIDATION 1
	#endif
#endif

// platform specific settings

#if defined(_WIN32)
	#define USE_PLATFORM_GLFW
	//#define VK_USE_PLATFORM_WIN32_KHR
	#define _CRT_SECURE_NO_WARNINGS
	//#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(__CYGWIN__)
	#define VK_USE_PLATFORM_WIN32_KHR
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(__MINGW32__)
	#define USE_PLATFORM_GLFW
	//#define VK_USE_PLATFORM_WIN32_KHR
	//#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(__linux__)
	#define USE_PLATFORM_GLFW
	//#define VK_USE_PLATFORM_XCB_KHR
#else
	//#error "Unsupported Vulkan WSI platform." // caught in main.cpp instead
#endif

TODO( "Add other (all would be awesome) platforms" )

#endif //COMMON_VULKAN_ENVIRONMENT_H
