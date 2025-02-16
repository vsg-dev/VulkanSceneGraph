cmake_minimum_required(VERSION 3.7)

project(vsg
    VERSION 1.1.11
    DESCRIPTION "VulkanSceneGraph library"
    LANGUAGES C CXX
)
set(VSG_SOVERSION 14)
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

# Set the instrumentation level to compile into sources
set(VSG_MAX_INSTRUMENTATION_LEVEL 1 CACHE STRING "Set the instrumentation level to build into the VSG ibrary, 0 for off, 1 coarse grained, 2 medium, 3 fine grained." )

# Enable/disable shader compilation support that pulls in glslang
set(VSG_SUPPORTS_ShaderCompiler  1 CACHE STRING "Optional shader compiler support, 0 for off, 1 for enabled." )
if (VSG_SUPPORTS_ShaderCompiler)

    # Try looking for glslang 15 first.
    set(GLSLANG_MIN_VERSION "15" CACHE STRING "glslang 14 is the earliest version that we think installs itself properly on all platforms. Other platforms may be able to use an earlier version")
    find_package(glslang ${GLSLANG_MIN_VERSION} CONFIG QUIET)

    if (NOT glslang_FOUND)
        # fallback to trying glslang 14.
        set(GLSLANG_MIN_VERSION "14")
        find_package(glslang ${GLSLANG_MIN_VERSION} CONFIG)
    endif()

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
    elseif (UNIX)
        if(BUILD_WAYLAND)
            set(FIND_DEPENDENCY_WINDOWING "find_package(PkgConfig REQUIRED)\npkg_check_modules(wayland-client REQUIRED IMPORTED_TARGET wayland-client)\npkg_check_modules(wayland-cursor REQUIRED IMPORTED_TARGET wayland-cursor)\npkg_check_modules(xkbcommon REQUIRED IMPORTED_TARGET xkbcommon)\n")
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(wayland-client REQUIRED IMPORTED_TARGET wayland-client)
            pkg_check_modules(wayland-cursor REQUIRED IMPORTED_TARGET wayland-cursor)
            pkg_check_modules(xkbcommon REQUIRED IMPORTED_TARGET xkbcommon)
            #generate Wayland source files
            execute_process(COMMAND pkg-config wayland-protocols --variable=pkgdatadir
                OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)

            execute_process(COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml ${PROJECT_SOURCE_DIR}/include/vsg/platform/wayland/wayland-xdg-shell-client-protocol.h)
            execute_process(COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml ${PROJECT_SOURCE_DIR}/src/vsg/platform/wayland/wayland-xdg-shell-protocol.c)
            execute_process(COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ${PROJECT_SOURCE_DIR}/include/vsg/platform/wayland/xdg-decoration-client.h)
            execute_process(COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ${PROJECT_SOURCE_DIR}/src/vsg/platform/wayland/xdg-decoration-client.c)
            execute_process(COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-output/xdg-output-unstable-v1.xml ${PROJECT_SOURCE_DIR}/include/vsg/platform/wayland/xdg-output-client.h)
            execute_process(COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-output/xdg-output-unstable-v1.xml ${PROJECT_SOURCE_DIR}/src/vsg/platform/wayland/xdg-output-client.c)
        else()
            set(FIND_DEPENDENCY_WINDOWING "find_package(PkgConfig REQUIRED)\npkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)\n")
            # just use Xcb for native windowing
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)
        endif()
    else()
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
set(PBR_DEFINES
    -v \"\"
    -v "VSG_DIFFUSE_MAP"
    -v "VSG_EMISSIVE_MAP"
    -v "VSG_LIGHTMAP_MAP"
    -v "VSG_METALLROUGHNESS_MAP"
    -v "VSG_NORMAL_MAP"
    -v "VSG_SKINNING"
    -v "VSG_TWO_SIDED_LIGHTING"
    -v "VSG_WORKFLOW_SPECGLOSS"
    -v "VSG_DIFFUSE_MAP VSG_EMISSIVE_MAP"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP"
    -v "VSG_DIFFUSE_MAP VSG_METALLROUGHNESS_MAP"
    -v "VSG_DIFFUSE_MAP VSG_NORMAL_MAP"
    -v "VSG_DIFFUSE_MAP VSG_SKINNING"
    -v "VSG_DIFFUSE_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_WORKFLOW_SPECGLOSS"
    -v "VSG_LIGHTMAP_MAP VSG_METALLROUGHNESS_MAP"
    -v "VSG_LIGHTMAP_MAP VSG_NORMAL_MAP"
    -v "VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP"
    -v "VSG_METALLROUGHNESS_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_EMISSIVE_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP VSG_METALLROUGHNESS_MAP"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP VSG_NORMAL_MAP"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP"
    -v "VSG_DIFFUSE_MAP VSG_METALLROUGHNESS_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_NORMAL_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_LIGHTMAP_MAP VSG_NORMAL_MAP VSG_WORKFLOW_SPECGLOSS"
    -v "VSG_DIFFUSE_MAP VSG_EMISSIVE_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP VSG_NORMAL_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_EMISSIVE_MAP VSG_LIGHTMAP_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP"
    -v "VSG_DIFFUSE_MAP VSG_EMISSIVE_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_LIGHTMAP_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP VSG_WORKFLOW_SPECGLOSS"
    -v "VSG_DIFFUSE_MAP VSG_EMISSIVE_MAP VSG_LIGHTMAP_MAP VSG_METALLROUGHNESS_MAP VSG_NORMAL_MAP VSG_TWO_SIDED_LIGHTING"
    -v "VSG_DIFFUSE_MAP VSG_EMISSIVE_MAP VSG_LIGHTMAP_MAP VSG_NORMAL_MAP VSG_SPECULAR_MAP VSG_WORKFLOW_SPECGLOSS"
    -v "VSG_DISPLACEMENT_MAP VSG_DIFFUSE_MAP"
    -v "VSG_DISPLACEMENT_MAP VSG_DETAIL_MAP VSG_DIFFUSE_MAP"
)

set(PHONG_DEFINES
    -v \"\"
    -v "VSG_BILLBOARD"
    -v "VSG_DIFFUSE_MAP"
    -v "VSG_BILLBOARD VSG_DIFFUSE_MAP"
    -v "VSG_DISPLACEMENT_MAP VSG_DIFFUSE_MAP"
    -v "VSG_DISPLACEMENT_MAP VSG_DETAIL_MAP VSG_DIFFUSE_MAP"
)

set(FLAT_DEFINES
    -v \"\"
    -v "VSG_BILLBOARD"
    -v "VSG_DIFFUSE_MAP"
    -v "VSG_BILLBOARD VSG_DIFFUSE_MAP"
    -v "VSG_DISPLACEMENT_MAP VSG_DIFFUSE_MAP"
    -v "VSG_DISPLACEMENT_MAP VSG_DETAIL_MAP VSG_DIFFUSE_MAP"
)

set(TEXT_DEFINES
    -v "CPU_LAYOUT"
    -v "CPU_LAYOUT BILLBOARD"
    -v "GPU_LAYOUT"
    -v "GPU_LAYOUT BILLBOARD"
)

add_custom_target(build_ShaderSets
    COMMAND vsgshaderset --pbr ${PBR_DEFINES} -o src/vsg/utils/shaders/pbr_ShaderSet.cpp --binary
    COMMAND vsgshaderset --phong ${PHONG_DEFINES}  -o src/vsg/utils/shaders/phong_ShaderSet.cpp --binary
    COMMAND vsgshaderset --flat ${FLAT_DEFINES} -o src/vsg/utils/shaders/flat_ShaderSet.cpp --binary
    COMMAND vsgshaderset --text ${TEXT_DEFINES}  -o src/vsg/text/shaders/text_ShaderSet.cpp  --binary
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
