#ifndef HELLO_TRIANGLE_WSI_PLATFORM_H
#define HELLO_TRIANGLE_WSI_PLATFORM_H

#if  !defined(USE_PLATFORM_GLFW) \
  && !defined(VK_USE_PLATFORM_ANDROID_KHR) \
  && !defined(VK_USE_PLATFORM_MIR_KHR) \
  && !defined(VK_USE_PLATFORM_WAYLAND_KHR) \
  && !defined(VK_USE_PLATFORM_WIN32_KHR) \
  && !defined(VK_USE_PLATFORM_XCB_KHR) \
  && !defined(VK_USE_PLATFORM_XLIB_KHR) \
  && !defined(VK_USE_PLATFORM_IOS_MVK) \
  && !defined(VK_USE_PLATFORM_MACOS_MVK) \
  && !defined(VK_USE_PLATFORM_VI_NN)
	#error "Exactly one Vulkan WSI platform must be defined."
#endif

#if defined(USE_PLATFORM_GLFW)
	#include "WSI/Glfw.h"
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	#include "WSI/Win32.h"
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	#include "WSI/Xlib.h"
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	#include "WSI/Xcb.h"
#else
	#error "Unsupported Vulkan WSI platform, or none selected."
#endif

#endif //HELLO_TRIANGLE_WSI_PLATFORM_H