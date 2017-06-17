Hello Triangle Vulkan demo
=========================

This is a traditional Hello World style application for graphical APIs (for
Vulkan in this case). It renders a RGB shaded equilateral triangle (well, if the
resolution is a square).

The code is quite flat and basic, so I think it's good enough for learning. No
tutorial or even much comments are provided though (comments do lie anyway
:smirk:).

But I have tried not to cut corners making it. It contains proper error handling
and I assume it is without errors :innocent:. It should
perfectly adhere to the Vulkan spec e.g. it should do proper synchronization and
do so in efficient way (as it was meant to be used). Well, at least that is the
goal (to have at least for this elementary case a flawless application).

 - TODO: check limits
 - TODO: make vertex buffer in best memory + investigate device memory alignment

Branches
-----------------

The git branches demonstrate some elementary Vulkan techniques which can be
easily grafted on this basic example (it is nice to see a diff of what needs
to be changed for it to work). Their `README.md` should be edited to reflect
the changes.

| branch | description |
|---|---|
| `MSAA` | Antialiasing (i.e. Vulkan's multisample image resolve) |
| `queue_transfer` | Transfer of `EXCLUSIVE` image between queue families (separate graphics and compute queue family) |
| `vertex_offset` | Demonstrates how offset in `vkCmdBindVertexBuffers()` works |

Proper renderloop synchronization mini-tutorial
-------------------------------------

Well, people tend to ask this one over and over, so I will make an exception to
the no tutorial policy above :wink::
[synchronizationTutorial.md](synchronizationTutorial.md).

Requirements
----------------------------

**OS**: Windows or Linux  
**Language**: C++14  
**Environment**: installed (latest) LunarG SDK  
**Environment**: preferably installed (latest) Vulkan capable drivers  
**Environment**: On Windows MS Visual Studio, Cygwin, MinGW (or IDEs running on top of
them)   
**Environment**: On Linux g++ and libxcb-dev and libxcb-keysyms-dev  
**Environment[Optional]**: Optionally GLFW 3.2+

TODO: Adding VkSurface function for other OSes should be straightforward though
using the provided ones as template for it.

Files
----------------------------------

| file | description |
|---|---|
| HelloTriangle.cpp | The app souce code including `main` function |
| VulkanEnvironment.h | Includes `vulkan.h` and the necessary platform headers |
| LeanWindowsEnvironment.h | Included conditionally by `VulkanEnvironment.h` and includes lean `windows.h` header |
| CompilerMessages.h | Allows to make compile-time messages shown in the compiler output |
| ErrorHandling.h | `VkResult` check helpers + `VK_EXT_debug_report` extension related stuff |
| WSI/Glfw.h | WSI platform-dependent stuff via GLFW3 library |
| WSI/Win32.h | WSI Win32 platform-dependent stuff |
| WSI/Xcb.h | WSI XCB platform-dependent stuff |
| WSI/Xlib.h | WSI XLIB platform-dependent stuff |
| Wsi.h | Meta-header choosing one of the platform in WSI directory |
| Vertex.h | Just simple Vertex definitions |
| EnumerateScheme.h | A scheme to unify usage of most Vulkan `vkEnumerate*` and `vkGet*` commands |
| ExtensionLoader.h | Functions handling loading of Vulkan extension commands |
| triangle.vert | The vertex shader program in GLSL |
| triangle.frag | The fragment shader program in GLSL |
| triangle.vert.spv | triangle.vert pre-transcripted to SPIR-V for convenience |
| triangle.frag.spv | triangle.frag pre-transcripted to SPIR-V for convenience |
| CMakeLists.txt | CMake makefile |

Config
---------------------------------------

You can change the application configuration by simply changing following
variables in `HelloTriangle.cpp`.

| config variable | purpose | default |
|---|---|---|
| `debugVulkan` | Turns debug output and validation layers on | tries to choose based on compiler debug mode |
| `debugAmount` | Which kinds of debug messages will be shown | `WARNING` \| `PERFORMANCE_WARNING` \| `ERROR` |
| `fpsCounter` | Enable FPS counter via `VK_LAYER_LUNARG_monitor` layer | `true` |
| `initialWindowWidth` | The initial width of the rendered window | `800` |
| `initialWindowHeight` | The initial height of the rendered window | `800` |
| `presentMode` | The presentation mode of Vulkan used in swapchain | `VK_PRESENT_MODE_FIFO_KHR` <sup>1</sup>|
| `clearColor` | Background color of the rendering | gray (`{0.1f, 0.1f, 0.1f, 1.0f}`) |
| `vertexShaderFilename` | The file with the SPIR-V vertex shader program | `triangle.vert.spv` |
| `fragmentShaderFilename` | The file with the SPIR-V fragment shader program | `triangle.frag.spv` |

<sup>1</sup> I preferred `VK_PRESENT_MODE_IMMEDIATE_KHR` before but it tends to
make coil whine because of the exteme FPS (which could be unnecessarily
dangerous to the PCB).

Build
----------------------------------------------

In many cases CMake style build should work just fine:

    $ cmake -G"Your generator"

In Cygwin you can build it e.g. thusly (for x64):

    $ g++ -std=c++14 -Wall -m64 -mwindows -Wl,--subsystem,console -D_DEBUG -I./ -I$VULKAN_SDK/Include -oHelloTriangle HelloTriangle.cpp -L$WINDIR/System32 -lvulkan-1

In MinGW like so:

    $ g++ -std=c++14 -Wall -m32 -mwindows -Wl,--subsystem,console -Wl,--allow-multiple-definition -D_DEBUG -I./ -I$VULKAN_SDK/Include -I/path/to/GLFW/ -oHelloTriangle HelloTriangle.cpp -L$VULKAN_SDK/Lib32 -lvulkan-1 -L/path/to/GLFW/ -lglfw3

In MS Visual Studio you can create Solution for it.  
You would add `$(VULKAN_SDK)\Include` to the the Additional Include Directories
and `$(VULKAN_SDK)\Bin\vulkan-1.lib` (or `Bin32` for x86) and `glfw3.lib` to the
Additional Dependencies property. You may choose between `windows` or `console`
subsystem in SubSystem property.

In Linux distro you would do e.g.:

    $ g++ --std=c++14 -Wall -m64 -D_DEBUG -DNO_TODO -I$VULKAN_SDK/include -oHelloTriangle HelloTriangle.cpp -ldl -L$VULKAN_SDK/lib -lvulkan -lglfw

There are annoying (on purpose) TODOs generated on build. They can be disabled
by defining `NO_TODO`.

Using GLFW is optional. You may choose another windowing platform in
`VulkanEnvironment.h`. All platforms use GLFW by default except Cygwin (which
uses Win32 directly to reduce dependencies on X11).

Run
------------------------

You just run it as you would anything else. You just need to make sure
`triangle.vert.spv` and `triangle.frag.spv` files are visible to the binary
(e.g. in the same folder).

<kbd>Esc</kbd> does terminate the app.  
<kbd>Alt</kbd> + <kbd>Enter</kbd> toggles fullscreen if using GLFW.
