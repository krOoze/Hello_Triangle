Hello Triangle Vulkan demo -- vertex binding offset
=========================

This is a traditional Hello World style application for the Vulkan API. It
renders a RGB shaded equilateral triangle (well, if the resolution is a square).
And it tries if vertex binding offset using `vkCmdBindVertexBuffers()` works.

The code is relatively flat and basic, so I think it's good enough for learning.
No tutorial or even much comments are provided though (comments do lie anyway
:smirk:). Though some things I found noteworthy are in the `doc/` folder.

My goal here is to be perfectly conformant to the Vulkan specification. Also I
do handle all the Vulkan `VkResult`s (with app termination though). And I try to
do things in the efficient way, rather than simplify. But I am only human.
:innocent:

If you appriciate any of my free work or help (e.g. hosting this), you can send
me some sweet sweet money:

<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=6NRLHTNEP8GH8&item_name=For+Petr+Kraus+%28aka+krOoze%29+if+you+appreciate+any+of+my+work+done+for+free.&currency_code=USD&source=url">
<img width="145" height="30" src="http://vulkan.hys.cz/donate.png" alt="Donate $ to krOoze" /></a>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=6NRLHTNEP8GH8&item_name=For+Petr+Kraus+%28aka+krOoze%29+if+you+appreciate+any+of+my+work+done+for+free.&currency_code=EUR&source=url">
<img width="145" height="30" src="http://vulkan.hys.cz/donateEur.png" alt="Donate € to krOoze" /></a>

Branches
-----------------

The git branches demonstrate some basic Vulkan features which can be easily
grafted on this basic example (it is nice to see a diff of what exactly needs to
be changed to make it work). Their `README.md` should be edited to reflect the
changes.

| branch | diff link | description |
|---|---|---|
| [master](https://github.com/krOoze/Hello_Triangle/tree/master)| -- | Your regular Hello Triangle app, and base for the others
| [MSAA](https://github.com/krOoze/Hello_Triangle/tree/MSAA)  | [diff](https://github.com/krOoze/Hello_Triangle/compare/MSAA) | Antialiasing (i.e. Vulkan's multisample image resolve) |
| [queue_transfer](https://github.com/krOoze/Hello_Triangle/tree/queue_transfer) | [diff](https://github.com/krOoze/Hello_Triangle/compare/queue_transfer) | Transfer of `EXCLUSIVE` image between queue families (separate graphics and compute queue family) |
| [vertex_offset](https://github.com/krOoze/Hello_Triangle/tree/vertex_offset) | [diff](https://github.com/krOoze/Hello_Triangle/compare/vertex_offset) | Demonstrates how offset in `vkCmdBindVertexBuffers()` works |
| [dxgi_interop](https://github.com/krOoze/Hello_Triangle/tree/dxgi_interop) | [diff](https://github.com/krOoze/Hello_Triangle/compare/dxgi_interop) | An experiment that shows how to import swapchain images from DXGI (Direct3D 12 Swapchain) |

Requirements
----------------------------

**OS**: Windows or Linux  
**Language**: C++14  
**Build environment**: (latest) [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (requires `VULKAN_SDK` variable being set)  
**Build environment[Windows]**: Visual Studio, Cygwin, or MinGW (or IDEs running on top of them)  
**Build environment[Linux]**: CMake compatible compiler and build system and `libxcb-dev` and `libxcb-keysyms-dev`  
**Build environment[MacOS]**: CMake compatible compiler and build system  
**Build Environment[GLFW]**: GLFW 3.2+ (already included as a git submodule), requires `xorg-dev` (or XCB) package on Linux  
**Build environment[Xlib]**: Requires `xorg-dev` package  
**Build environment[XCB]**: Requires `libxcb1-dev`, `libxcb-util-dev`, `libxcb-keysyms1-dev`, and `x11proto-dev` packages  
**Build environment[Wayland]**: Requires `libwayland-dev` and `libxkbcommon-dev` packages  
**Target Environment**: installed (latest) Vulkan capable drivers (to see anything)  
**Target Environment**: GLFW(recommended), XCB, Xlib, or Wayland based windowing system

On Unix-like environment refer to
[SDK docs](https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started.html)
on how to set `VULKAN_SDK` variable.

Adding `VkSurface` support for other platforms should be straightforward using
the provided ones as a template for it.

Files
----------------------------------

| file | description |
|---|---|
| [doc/synchronizationTutorial.md](doc/synchronizationTutorial.md) | Short explanation of Vulkan synchronization in the context of trivial applications |
| [doc/Schema.pdf](doc/Schema.pdf) | Bunch of diagrams explaining the app architecture and synchronization in a graphical form |
| external/glfw/ | GLFW git submodule |
| src/HelloTriangle.cpp | The app souce code, including the `main()` function |
| src/CompilerMessages.h | Allows to make compile-time messages shown in the compiler output |
| src/EnumerateScheme.h | A scheme to unify usage of most Vulkan `vkEnumerate*` and `vkGet*` commands |
| src/ErrorHandling.h | `VkResult` check helpers + `VK_EXT_debug_utils` extension related stuff |
| src/ExtensionLoader.h | Functions handling loading of select Vulkan extension commands |
| src/LeanWindowsEnvironment.h | Included conditionally by `VulkanEnvironment.h` and includes lean `windows.h` header |
| src/Vertex.h | Just simple Vertex definitions |
| src/VulkanEnvironment.h | Contains header configuration, such platform-specific as `VK_USE_PLATFORM_*` |
| src/VulkanIntrospection.h | Introspection of Vulkan entities; e.g. convert Vulkan enumerants to strings |
| src/Wsi.h | Meta-header including one of the platform-specific headers in WSI directory |
| src/WSI/Glfw.h | WSI platform-dependent stuff via GLFW3 library |
| src/WSI/Win32.h | Win32 WSI platform-dependent stuff |
| src/WSI/Xcb.h | XCB WSI platform-dependent stuff |
| src/WSI/Xlib.h | Xlib WSI platform-dependent stuff |
| src/WSI/Wayland.h | Wayland WSI platform-dependent stuff |
| src/WSI/private/ | Stuff the WSI headers need; currently just generated Wayland protocols |
| src/shaders/hello_triangle.vert | The vertex shader program in GLSL |
| src/shaders/hello_triangle.frag | The fragment shader program in GLSL |
| .gitignore | Git filter file ignoring most probable outputs messing up the local repo |
| .gitmodules | Git submodules file describing the dependency on GLFW |
| CMakeLists.txt | CMake makefile |
| CONTRIBUTING.md | Extra info about contributing code, ideas, and bug reports |
| LICENSE | Copyright licence for this project |
| README.md | This file |

Config
---------------------------------------

You can change the application configuration by simply changing following
variables in `HelloTriangle.cpp`.

| config variable | purpose | default |
|---|---|---|
| `appName` | Application name; might show up in title of the window | whatever it is in code |
| `debugSeverity` | Which kinds of debug message severites will be shown | `WARNING` \| `ERROR` |
| `debugType` | Which kinds of debug message types will be shown | all types |
| `useAssistantLayer` | Enable Assistant Layer too when debugging (TODO: does not support new way of enabling) | `false` |
| `fpsCounter` | Enable FPS counter via `VK_LAYER_LUNARG_monitor` layer | `true` |
| `initialWindowWidth` | The initial width of the rendered window | `800` |
| `initialWindowHeight` | The initial height of the rendered window | `800` |
| `presentMode` | The presentation mode of Vulkan used in swapchain | `VK_PRESENT_MODE_FIFO_KHR` <sup>1</sup>|
| `clearColor` | Background color of the rendering | gray (`{0.1f, 0.1f, 0.1f, 1.0f}`) |
| `forceSeparatePresentQueue` | By default the app prioritizes single Graphics and Present queue. This will create separate queues for testing purposes. There are virtually no platforms currently that naturally have separate Present queue family. |

<sup>1</sup> I preferred `VK_PRESENT_MODE_IMMEDIATE_KHR` before but it tends to
make coil whine because of the extreme FPS (which could be unnecessarily
dangerous to the PCB in the long term).

Build
----------------------------------------------

First just get everything:

    $ git clone --recurse-submodules https://github.com/krOoze/Hello_Triangle.git

In many platforms CMake style build should work just fine:

    $ cmake -G"Your preferred generator"

or even just

    $ cmake .

Then use `make`, or the generated Visual Studio `*.sln`, or whatever it created.

There are two cmake options (supplied by `-D`):
 - `WSI` -- set this to `USE_PLATFORM_GLFW` or any `VK_USE_PLATFORM_*_KHR` to
    select the WSI to be used. Default is GLFW.
 - `TODO` -- set this to `OFF` to remove TODO messages during compilation.

You also might want to add `-DCMAKE_BUILD_TYPE=Debug`.

Manual Build
----------------------------------------------

The code base is quite simple and it can be built manually. It only assumes that
the `src` folder is in the include path, that you link the GLFW dependency, and
that you compile the GLSL shader files into SPIR-V.

In Visual Studio you can simply import the code and add the things mentioned
above. Either "console" or "window" subsystem is a valid choice on Windows.

In Linux distro you would do e.g.:

    $ g++ --std=c++14 -Wall -m64 -D_DEBUG -DNO_TODO -I$VULKAN_SDK/include -I./src/ -o./HelloTriangle src/HelloTriangle.cpp -ldl -L$VULKAN_SDK/lib -lvulkan -lglfw

GLSL shaders can be compiled using `glslc` from LunarG Vulkan SDK like so:  
On Windows-like environment:

    %VULKAN_SDK%/Bin/glslc -mfmt=c -o ./src/shaders/hello_triangle.vert.spv.inl ./src/shaders/hello_triangle.vert
    %VULKAN_SDK%/Bin/glslc -mfmt=c -o ./src/shaders/hello_triangle.frag.spv.inl ./src/shaders/hello_triangle.frag

Or on Unix-like environment you would use just `$VULKAN_SDK` instead:

    $VULKAN_SDK/Bin/glslc -mfmt=c -o ./src/shaders/hello_triangle.vert.spv.inl ./src/shaders/hello_triangle.vert
    $VULKAN_SDK/Bin/glslc -mfmt=c -o ./src/shaders/hello_triangle.frag.spv.inl ./src/shaders/hello_triangle.frag

There are annoying (on purpose) TODOs generated on build. They can be disabled
by defining `NO_TODO` preprocessor macro.

Using GLFW is optional. You may choose another windowing platform by modifying
`VulkanEnvironment.h`, or supplying it via preprocessor. By default, all
platforms use GLFW.

Run
------------------------

You just run it as you would anything else.

<kbd>Esc</kbd> does terminate the app.  
<kbd>Alt</kbd> + <kbd>Enter</kbd> toggles fullscreen (might not work on some WSI
platforms).
