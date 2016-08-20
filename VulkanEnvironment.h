// vulkan.h header global settings

// Should be included as soon as possible (first line in translation unit preferably)

#ifndef COMMON_VULKAN_ENVIRONMENT_H
#define COMMON_VULKAN_ENVIRONMENT_H

#ifdef VULKAN_H_
	#error Some vulkan.h is already included before this file!
#endif //VULKAN_H_

#include "CompilerMessages.h"

#ifdef _WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#elif __CYGWIN__
	#define VK_USE_PLATFORM_WIN32_KHR
#elif  __MINGW32__
	#define VK_USE_PLATFORM_WIN32_KHR
#else
	#error "Unsupported platform"
#endif

// Windows.h settings must be first -- vulkan.h does include Windows.h
#ifdef VK_USE_PLATFORM_WIN32_KHR
	#include "LeanWindowsEnvironment.h"
#endif

TODO( "Add other platforms" )

#endif //COMMON_VULKAN_ENVIRONMENT_H