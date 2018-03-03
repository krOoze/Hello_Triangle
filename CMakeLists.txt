cmake_minimum_required( VERSION 3.5.1 )
project( HelloTriangle )

if( NOT DEFINED ENV{VULKAN_SDK} )
	message( FATAL_ERROR "VULKAN_SDK not found!" )
endif()

if( CYGWIN )
	execute_process( COMMAND cygpath "$ENV{VULKAN_SDK}" OUTPUT_VARIABLE VULKAN_SDK )
	string( STRIP ${VULKAN_SDK} VULKAN_SDK )
else()
	set( VULKAN_SDK "$ENV{VULKAN_SDK}" )
endif()
message( "Vulkan SDK path: " ${VULKAN_SDK} )

# Shaders
set( DEBUG_FLAG $<$<CONFIG:Debug>:-g> )
set( GLSL_COMPILER ${VULKAN_SDK}/bin/glslc -mfmt=num ${DEBUG_FLAG} )

set(VERT_SHADER "src/shaders/hello_triangle.vert")
set(VERT_SHADER_INCLUDE ${VERT_SHADER}.spv.inl)
add_custom_command(
	COMMENT "Compiling vertex shader"
	MAIN_DEPENDENCY ${VERT_SHADER}
	OUTPUT ${VERT_SHADER_INCLUDE}
	COMMAND ${GLSL_COMPILER} -o ${VERT_SHADER_INCLUDE} ${VERT_SHADER}
	#VERBATIM -- TODO breaks empty generator-expression
)

set(FRAG_SHADER "src/shaders/hello_triangle.frag")
set(FRAG_SHADER_INCLUDE ${FRAG_SHADER}.spv.inl)
add_custom_command(
	COMMENT "Compiling fragment shader"
	MAIN_DEPENDENCY ${FRAG_SHADER}
	OUTPUT ${FRAG_SHADER_INCLUDE}
	COMMAND ${GLSL_COMPILER} -o ${FRAG_SHADER_INCLUDE} ${FRAG_SHADER}
	#VERBATIM -- TODO breaks empty generator-expression
)

add_custom_target(
	HelloTriangle_shaders
	COMMENT "Compiling shaders"
	DEPENDS ${VERT_SHADER_INCLUDE} ${FRAG_SHADER_INCLUDE}
)

# GLFW
set( GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE )
set( GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE )
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE )

if( CYGWIN )
	# Hack to make GLFW use Win32 WSI instead of X11
	# TODO: Might be cleaner to use cross-compiling
	unset( UNIX )
	set( WIN32 1 )
endif()

add_subdirectory( external/glfw )

# HelloTriangle binary
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT HelloTriangle )
file(GLOB SOURCE_HEADERS "src/*.h")
add_executable( HelloTriangle WIN32 src/HelloTriangle.cpp ${SOURCE_HEADERS} )
add_dependencies( HelloTriangle HelloTriangle_shaders )

if( MSVC )
	target_compile_definitions( HelloTriangle PRIVATE $<$<CONFIG:Debug>:_CONSOLE> )
	set_target_properties( HelloTriangle PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE" )
endif()

if("${CMAKE_VERSION}" VERSION_LESS 3.8.2)
	set_target_properties(
		HelloTriangle
		PROPERTIES
			CXX_STANDARD 14
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO
	)
else()
	target_compile_features( HelloTriangle PRIVATE cxx_std_14 )
endif()

find_path(
	VULKAN_INCLUDE vulkan/vulkan.h
	PATHS "${VULKAN_SDK}/Include" "${VULKAN_SDK}/include"
	NO_DEFAULT_PATH
)
message( "Vulkan include dir: " ${VULKAN_INCLUDE} )
include_directories( "${VULKAN_INCLUDE}" "src/" )

if( CYGWIN )
	set( CMAKE_FIND_LIBRARY_PREFIXES "" )
	set( CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll" )
endif()
find_library(
	VULKAN_LIBRARY
	NAMES vulkan vulkan-1
	PATHS "${VULKAN_SDK}/lib" "${VULKAN_SDK}/Lib" "${VULKAN_SDK}/Lib32"
	NO_DEFAULT_PATH
)
message( "Vulkan libs: " ${VULKAN_LIBRARY} )
if( UNIX AND NOT CYGWIN AND NOT MINGW )
	set( ADDITIONAL_LIBS xcb xcb-keysyms X11 )
endif()
target_link_libraries( HelloTriangle "${VULKAN_LIBRARY}" glfw ${ADDITIONAL_LIBS} )
