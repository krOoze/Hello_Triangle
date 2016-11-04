// vulkan.h header global settings

// Should be included as soon as possible (first line in translation unit preferably)

#ifndef COMMON_VULKAN_ENVIRONMENT_H
#define COMMON_VULKAN_ENVIRONMENT_H

// detect premature vulkan.h
#ifdef VULKAN_H_
	#error Some vulkan.h is already included before this file!
#endif //VULKAN_H_

#include "CompilerMessages.h"

// platform specific settings
TODO( "Apparently XCB can exist in windows compiler, so shouldn't be choosing automatically." )

#if defined(_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR
	#define _CRT_SECURE_NO_WARNINGS
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(__CYGWIN__)
	#define VK_USE_PLATFORM_WIN32_KHR
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(__MINGW32__)
	#define VK_USE_PLATFORM_WIN32_KHR
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif defined(__linux__)
	#define VK_USE_PLATFORM_XLIB_KHR
#else
	#error "Unsupported platform"
#endif

TODO( "Add other (all would be awesome) platforms" )

#endif //COMMON_VULKAN_ENVIRONMENT_H