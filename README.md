Hello Triangle Vulkan demo
=========================

This is a traditional Hello World style application for graphical APIs (for Vulkan in this case).
It renders a RGB shaded equilateral triangle.

The code is quite flat and basic, so I think it's good enough for learning.
No tutorial or even comments are provided though (comments do lie anyway :smirk:).

But I have tried not to cut corners making it. It contains proper error handling
and I assume it is without errors :innocent:. It should perfectly adhere to the Vulkan spec
e.g. it should do proper synchronization and do so in efficient way (as it was meant to be used).

TODO: check features and limits

Requirements
----------------------------

**OS**: Windows  
**Language**: C++14  
**Environment**: installed LunarG SDK  
**Environment**: preferably installed Vulkan capable drivers  
**Environment**: MS Visual Studio or Cygwin  

Adding VkSurface function for other OSes should be straightforward though.
I would welcome if someone PR'd it (unless I do first :smile:).

TODO: make sure it works in MinGW too.

Files
----------------------------------

| file | description |
|---|---|
| HelloTriangle.cpp | The app souce code including `main` function |
| VulkanEnvironment.h | Includes `vulkan.h` and the necessary platform headers |
| LeanWindowsEnvironment.h | Included by `VulkanEnvironment.h` and includes lean `windows.h` header |
| triangle.vert | The vertex shader program in GLSL |
| triangle.frag | The fragment shader program in GLSL |
| triangle.vert.spv | triangle.vert pre-transcripted to SPIR-V for convenience |
| triangle.frag.spv |  triangle.frag pre-transcripted to SPIR-V for convenience |

Config
---------------------------------------

You can change the application configuration by simply changing following
variables in `HelloTriangle.cpp`.

| config variable | purpose |
|---|---|
| `debugVulkan` | Turns debug output and validation layers on |
| `debugAmount` | Which kinds of debug messages will be shown |
| `windowWidth` | The width of the rendered window |
| `windowHeight` | The height of the rendered window |
| `presentMode` | The presentation mode of Vulkan used in swapchain |
| `clearColor` | Background color of the rendering |
| `vertexShaderFilename` | The file with the SPIR-V vertex shader program |
| `fragmentShaderFilename` | The file with the SPIR-V fragment shader program |

Build
----------------------------------------------

In Cygwin you can build it e.g. thusly (for x64):

    $ g++ -std=c++14 -Wall -m64 -mwindows -Wl,--subsystem,console -I$VULKAN_SDK/Include -oHelloTriangle HelloTriangle.cpp -L$WINDIR/System32 -lvulkan-1

In MS Visual Studio you can create Solution for it.
You would add `$(VULKAN_SDK)\Include` to the the Additional Include Directories and
`$(VULKAN_SDK)\Bin\vulkan-1.lib` (or `Bin32` for x86) to the Additional Dependencies property.
You may choose between `windows` or `console` subsystem in SubSystem property.

Run
------------------------

You just run it as you would anything else. You just need to make sure `triangle.vert.spv`
and `triangle.frag.spv` files are visible to the binary (e.g. in the same folder).
