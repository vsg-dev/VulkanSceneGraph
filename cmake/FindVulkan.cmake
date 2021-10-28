#
# wrapper for finding a specific minimum Vulkan version
#

if(Vulkan_FIND_REQUIRED)
    set(OPTIONS REQUIRED)
endif()

# force using cmake provided module path
set(_SAVE_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
unset(CMAKE_MODULE_PATH)

#
# For cmake versions < 3.23, the included FindVulkan.cmake
# module has no version support.
#
if(CMAKE_VERSION VERSION_LESS "3.23")
    find_package(Vulkan ${Vulkan_FIND_VERSION} ${OPTIONS} QUIET)
    if(Vulkan_FIND_VERSION)
        message(STATUS "Using Vulkan version check embedded in vsg")
        vsg_check_min_vulkan_header_version(${Vulkan_FIND_VERSION})
    endif()
else()
    find_package(Vulkan ${Vulkan_FIND_VERSION} ${OPTIONS})
endif()

# restore cmake module path
set(CMAKE_MODULE_PATH ${_SAVE_CMAKE_MODULE_PATH})
