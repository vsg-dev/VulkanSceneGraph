cmake_minimum_required(VERSION 3.7)

project(vsg
    VERSION 0.1.5
    DESCRIPTION "VulkanSceneGraph library"
    LANGUAGES CXX
)
set(VSG_SOVERSION 2)
SET(VSG_RELEASE_CANDIDATE 0)

# create the version header
set(VSG_MAX_DEVICES 1 CACHE STRING "Set maximum number of vk/vsg::Device that are supported.")
set(VSG_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/vsg/core/Version.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/vsg/core/Version.h.in" "${VSG_VERSION_HEADER}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(vsgMacros)


SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

setup_dir_vars()

# Change the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(VSG_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wmaybe-uninitialized -Wshadow -Wunused -Wno-misleading-indentation -Wextra)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(VSG_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wshadow -Wunused -Wextra)
endif()

set(VSG_WARNING_FLAGS ${VSG_WARNING_FLAGS} CACHE STRING "Compiler flags to use." FORCE)
add_compile_options(${VSG_WARNING_FLAGS})

if (MSVC)
    # Increase Number of Sections in .Obj file
    add_compile_options(/bigobj)
endif()

if (VULKAN_SDK)
    message("Within VulkanSceneGraph cmake, setting VULKAN_SDK env var to " ${VULKAN_SDK})
    set(ENV{VULKAN_SDK} ${VULKAN_SDK})
endif()

find_package(Vulkan)
find_package(Threads)

if (NOT DEFINED glslang_FOUND)
    find_package(vsg_glslang REQUIRED CONFIG PATHS ${CMAKE_SOURCE_DIR}/src/vsg)
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
    find_package(PkgConfig)
    pkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)
endif()

add_target_docs(
    FILES
        ${CMAKE_SOURCE_DIR}/include/vsg
)

# automatically buil_all_h build target to generate the include/vsg/all.h from the headers in the include/vsg/* directories
add_custom_target(build_all_h
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/build/build_all_h.cmake
)

add_target_clobber()
add_target_uninstall()
add_target_cppcheck(
    SUPPRESSIONS_LIST
        "${CMAKE_SOURCE_DIR}/build/cppcheck-suppression-list.txt"
    FILES
        ${CMAKE_SOURCE_DIR}/include/vsg/*/*.h
        ${CMAKE_SOURCE_DIR}/src/vsg/*/*.cpp
        -I ${CMAKE_SOURCE_DIR}/include/
)

add_target_clang_format(
    FILES
        ${CMAKE_SOURCE_DIR}/include/vsg/*/*.h
        ${CMAKE_SOURCE_DIR}/src/vsg/*/*.cpp
)

add_option_maintainer(
    PREFIX VulkanSceneGraph
    RCLEVEL ${VSG_RELEASE_CANDIDATE}
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/vsgMacros.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vsg)

#
# src directory contains all the source of the vsg library
#
add_subdirectory(src/vsg)
