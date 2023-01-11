cmake_minimum_required(VERSION 3.7)

project(vsg
    VERSION 1.0.3
    DESCRIPTION "VulkanSceneGraph library"
    LANGUAGES CXX
)
set(VSG_SOVERSION 11)
SET(VSG_RELEASE_CANDIDATE 1)
set(Vulkan_MIN_VERSION 1.1.70.0)

set(VSG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Root source directory of VulkanSceneGraph.")
set(VSG_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "Root binary directory of VulkanSceneGraph.")

# create the version header
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

if (NOT DEFINED glslang_FOUND)
    find_package(vsg_glslang CONFIG PATHS ${VSG_SOURCE_DIR}/src/vsg)
endif()

if (glslang_FOUND)
    set(VSG_SUPPORTS_ShaderCompiler 1)
else()
    set(VSG_SUPPORTS_ShaderCompiler 0)
    message("--   Support for compiling GLSL to SPIR-V and associated shader composition is not available.")
endif()

if (ANDROID)
    find_library(AndroidLib android)
    if(CMAKE_SYSTEM_VERSION GREATER 24)
        find_library(AndroidNativeWindowLib nativewindow)
    endif()
elseif (WIN32)
    # just use native windowing
elseif (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
else()
    # just use Xcb for native windowing
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)
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

# automatically buil_all_h build target to generate the include/vsg/all.h from the headers in the include/vsg/* directories
add_custom_target(build_all_h
    COMMAND ${CMAKE_COMMAND} -DVSG_SOURCE_DIR=${VSG_SOURCE_DIR} -P ${VSG_SOURCE_DIR}/cmake/build_all_h.cmake
)
set_target_properties(build_all_h PROPERTIES FOLDER "VulkanSceneGraph")

vsg_add_option_maintainer(
    PREFIX VulkanSceneGraph
    RCLEVEL ${VSG_RELEASE_CANDIDATE}
)

#
# src directory contains all the source of the vsg library
#
add_subdirectory(src/vsg)

vsg_add_feature_summary()
