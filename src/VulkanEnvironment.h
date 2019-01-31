// vulkan.h header global settings

// Should be included as soon as possible (first line in translation unit preferably)

#ifndef COMMON_VULKAN_ENVIRONMENT_H
#define COMMON_VULKAN_ENVIRONMENT_H

// detect premature vulkan.h
#ifdef VULKAN_H_
	#error "vulkan.h is included before VulkanEnvironment.h!"
#endif //VULKAN_H_

#include "CompilerMessages.h"

#define REQUIRED_HEADER_VERSION 92

#ifdef NDEBUG
	#ifndef VULKAN_VALIDATION
		#define VULKAN_VALIDATION 0
	#endif
#else
	#ifndef VULKAN_VALIDATION
		#define VULKAN_VALIDATION 1
	#endif
#endif

#if  defined(USE_PLATFORM_GLFW) \
  || defined(VK_USE_PLATFORM_ANDROID_KHR) \
  || defined(VK_USE_PLATFORM_MIR_KHR) \
  || defined(VK_USE_PLATFORM_WAYLAND_KHR) \
  || defined(VK_USE_PLATFORM_WIN32_KHR) \
  || defined(VK_USE_PLATFORM_XCB_KHR) \
  || defined(VK_USE_PLATFORM_XLIB_KHR) \
  || defined(VK_USE_PLATFORM_IOS_MVK) \
  || defined(VK_USE_PLATFORM_MACOS_MVK) \
  || defined(VK_USE_PLATFORM_VI_NN)
	#define WSI_CHOSEN_EXTERNALLY
#endif

#if defined(_WIN32) && !defined(_CONSOLE)
	#include "LeanWindowsEnvironment.h"
	#include <Windows.h>
#endif

// platform specific settings
#if defined(__CYGWIN__)
	#ifndef WSI_CHOSEN_EXTERNALLY
		#define USE_PLATFORM_GLFW
		//#define VK_USE_PLATFORM_WIN32_KHR
	#endif
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(__MINGW32__)
	#ifndef WSI_CHOSEN_EXTERNALLY
		#define USE_PLATFORM_GLFW
		//#define VK_USE_PLATFORM_WIN32_KHR
	#endif
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(_WIN32)
	#ifndef WSI_CHOSEN_EXTERNALLY
		#define USE_PLATFORM_GLFW
		//#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#elif defined(__linux__)
	#ifndef WSI_CHOSEN_EXTERNALLY
		#define USE_PLATFORM_GLFW
		//#define VK_USE_PLATFORM_XCB_KHR
		//#define VK_USE_PLATFORM_XLIB_KHR
	#endif
#else
	//#error "Unsupported OS platform." // caught in main.cpp instead
#endif

TODO( "Add other (all would be awesome) platforms" )

#endif //COMMON_VULKAN_ENVIRONMENT_H
