// Includes Vulkan and OS Platform

#ifndef COMMON_VULKAN_ENVIRONMENT_H
#define COMMON_VULKAN_ENVIRONMENT_H

#ifdef VK_USE_PLATFORM_WIN32_KHR
	#include "LeanWindowsEnvironment.h"
#endif

// TODO: other platforms

#include <vulkan/vulkan.h> // includes vk_platform.h (stddef.h, stdint.h) and OS (e.g. windows.h)

#endif //COMMON_VULKAN_ENVIRONMENT_H