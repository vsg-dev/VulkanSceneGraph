cmake_minimum_required(VERSION 3.7)

project(vsg
    VERSION 1.1.4
    DESCRIPTION "VulkanSceneGraph library"
    LANGUAGES CXX
)
set(VSG_SOVERSION 14)
SET(VSG_RELEASE_CANDIDATE 0)
set(Vulkan_MIN_VERSION 1.1.70.0)

set(VSG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Root source directory of VulkanSceneGraph.")
set(VSG_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "Root binary directory of VulkanSceneGraph.")

# create the version heder
set(VSG_MAX_DEVICES 1 CACHE STRING "Set maximum number of vk/vsg::Device that are supported.")
set(VSG_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/vsg/core/Version.h")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(vsgMacros)

vsg_setup_dir_vars()
vsg_setup_build_vars()


if (MSVC)
    # Increase Number of Sections in .Obj file
    add_compile_options(/bigobj)
endif()

if (VULKAN_SDK)
    message("Within VulkanSceneGraph cmake, setting VULKAN_SDK env var to " ${VULKAN_SDK})
    set(ENV{VULKAN_SDK} ${VULKAN_SDK})
endif()

find_package(Vulkan ${Vulkan_MIN_VERSION} REQUIRED)

find_package(Threads REQUIRED)

# Set the instrumentation level to compile into sources
set(VSG_MAX_INSTRUMENTATION_LEVEL 1 CACHE STRING "Set the instrumentation level to build into the VSG ibrary, 0 for off, 1 coarse grained, 2 medium, 3 fine grained." )

# Enable/disable shader compilation support that pulls in glslang
set(VSG_SUPPORTS_ShaderCompiler  1 CACHE STRING "Optional shader compiler support, 0 for off, 1 for enabled." )
if (VSG_SUPPORTS_ShaderCompiler)
    set(GLSLANG_MIN_VERSION "14" CACHE STRING "glslang 14 is the earliest version that we think installs itself properly on all platforms. Other platforms may be able to use an earlier version")
    find_package(glslang ${GLSLANG_MIN_VERSION} CONFIG)

    if (glslang_FOUND)
        set(FIND_DEPENDENCY_glslang "find_package(glslang ${GLSLANG_MIN_VERSION} CONFIG REQUIRED)")
    else()
        message(WARNING "glslang not found. ShaderCompile support disabled.")
        set(VSG_SUPPORTS_ShaderCompiler 0)
        set(FIND_DEPENDENCY_glslang "")
    endif()
else()
    set(FIND_DEPENDENCY_glslang "")
endif()

set(VSG_SUPPORTS_Windowing  1 CACHE STRING "Optional native windowing support providing a default implementation of vsg::Window::create(), 0 for off, 1 for enabled." )
if (VSG_SUPPORTS_Windowing)
    if (ANDROID)
        set(FIND_DEPENDENCY_WINDOWING "")
        find_library(AndroidLib android)
        if(CMAKE_SYSTEM_VERSION GREATER 24)
            find_library(AndroidNativeWindowLib nativewindow)
        endif()
    elseif (WIN32)
        set(FIND_DEPENDENCY_WINDOWING "")
        # just use native windowing
    elseif (APPLE)
        set(FIND_DEPENDENCY_WINDOWING "find_library(COCOA_LIBRARY Cocoa)\nfind_library(QUARTZCORE_LIBRARY QuartzCore)\n")
        find_library(COCOA_LIBRARY Cocoa)
        find_library(QUARTZCORE_LIBRARY QuartzCore)
    else()
        set(FIND_DEPENDENCY_WINDOWING "find_package(PkgConfig REQUIRED)\npkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)\n")
        # just use Xcb for native windowing
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)
    endif()
endif()

# this line needs to be after the call to setup_build_vars()
configure_file("${VSG_SOURCE_DIR}/src/vsg/core/Version.h.in" "${VSG_VERSION_HEADER}")

vsg_add_target_clang_format(
    FILES
        ${VSG_SOURCE_DIR}/include/vsg/*/*.h
        ${VSG_SOURCE_DIR}/src/vsg/*/*.cpp
        ${VSG_SOURCE_DIR}/include/vsg/platform/unix/*.h
        ${VSG_SOURCE_DIR}/src/vsg/platform/unix/*.cpp
        ${VSG_SOURCE_DIR}/include/vsg/platform/win32/*.h
        ${VSG_SOURCE_DIR}/src/vsg/platform/win32/*.cpp
        ${VSG_SOURCE_DIR}/include/vsg/platform/android/*.h
        ${VSG_SOURCE_DIR}/src/vsg/platform/android/*.cpp
)
vsg_add_target_clobber()
vsg_add_target_cppcheck(
    SUPPRESSIONS_LIST
        "${VSG_SOURCE_DIR}/cmake/cppcheck-suppression-list.txt"
    FILES
        ${CMAKE_SOURCE_DIR}/include/vsg/*/*.h
        ${VSG_SOURCE_DIR}/src/vsg/*/*.cpp
        -I ${VSG_SOURCE_DIR}/include/
)
vsg_add_target_uninstall()
vsg_add_target_docs(
    FILES
        ${VSG_SOURCE_DIR}/include/vsg
)

# build_all_h build target to automatically generate the include/vsg/all.h from the headers in the include/vsg/* directories
add_custom_target(build_all_h
    COMMAND ${CMAKE_COMMAND} -DVSG_SOURCE_DIR=${VSG_SOURCE_DIR} -P ${VSG_SOURCE_DIR}/cmake/build_all_h.cmake
    COMMENT "update all.h to include all the include/vsg/*/*.h"
)
set_target_properties(build_all_h PROPERTIES FOLDER "VulkanSceneGraph")

# build_ShaderSets target automatically rebuilds the various built-in ShaderSets.
add_custom_target(build_ShaderSets
    COMMAND find ~/Data/glTF-Sample-Models/2.0 -name "*.glb" -o -name "*.gltf" | xargs vsgshaderset --pbr -o src/vsg/utils/shaders/pbr_ShaderSet.cpp --binary
    COMMAND vsgshaderset --phong -v \"\" -v "VSG_BILLBOARD" -v "VSG_DIFFUSE_MAP" -v "VSG_BILLBOARD VSG_DIFFUSE_MAP" -o src/vsg/utils/shaders/phong_ShaderSet.cpp --binary
    COMMAND vsgshaderset --flat -v \"\" -v "VSG_BILLBOARD" -v "VSG_DIFFUSE_MAP" -v "VSG_BILLBOARD VSG_DIFFUSE_MAP" -o src/vsg/utils/shaders/flat_ShaderSet.cpp --binary
    COMMAND vsgshaderset --text -v "CPU_LAYOUT" -v "CPU_LAYOUT BILLBOARD" -v "GPU_LAYOUT" -v "GPU_LAYOUT BILLBOARD" -o src/vsg/text/shaders/text_ShaderSet.cpp  --binary
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "update built-in ShaderSets"
)
add_custom_target(build_simple_ShaderSets
    COMMAND vsgshaderset --pbr -o src/vsg/utils/shaders/pbr_ShaderSet.cpp
    COMMAND vsgshaderset --phong -o src/vsg/utils/shaders/phong_ShaderSet.cpp
    COMMAND vsgshaderset --flat -o src/vsg/utils/shaders/flat_ShaderSet.cpp
    COMMAND vsgshaderset --text -o src/vsg/text/shaders/text_ShaderSet.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "update built-in ShaderSets"
)

vsg_add_option_maintainer(
    PREFIX v
    RCLEVEL ${VSG_RELEASE_CANDIDATE}
)

#
# src directory contains all the source of the vsg library
#
add_subdirectory(src/vsg)

vsg_add_feature_summary()
