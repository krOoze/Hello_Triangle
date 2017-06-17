cmake_minimum_required( VERSION 3.7.2 )
project( HelloTriangle CXX )

add_executable( HelloTriangle HelloTriangle.cpp )
add_definitions(-D_CONSOLE)
#target_compile_features( HelloTriangle PRIVATE cxx_std_14 )
set_property(TARGET HelloTriangle PROPERTY CXX_STANDARD 14)

find_package( Vulkan REQUIRED )
include_directories( ${Vulkan_INCLUDE_DIRS} )

set( GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE )
set( GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE )
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE )
add_subdirectory( external/glfw )

target_link_libraries( HelloTriangle ${Vulkan_LIBRARIES} glfw )
