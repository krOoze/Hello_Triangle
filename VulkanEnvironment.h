// vulkan.h header global settings

// Should be included as soon as possible (first line in translation unit preferably)

#ifndef COMMON_VULKAN_ENVIRONMENT_H
#define COMMON_VULKAN_ENVIRONMENT_H

// detect premature vulkan.h
#ifdef VULKAN_H_
	#error Some vulkan.h is already included before this file!
#endif //VULKAN_H_

// platform specific settings
#ifdef _WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
	#define _CRT_SECURE_NO_WARNINGS
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif __CYGWIN__
	#define VK_USE_PLATFORM_WIN32_KHR
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#elif  __MINGW32__
	#define VK_USE_PLATFORM_WIN32_KHR
	#include "LeanWindowsEnvironment.h" // Windows.h settings must be first -- vulkan.h does include Windows.h
#else
	#error "Unsupported platform"
#endif

#include "CompilerMessages.h"
TODO( "Add other (all would be awesome) platforms" )

#endif //COMMON_VULKAN_ENVIRONMENT_H