include(CMakeFindDependencyMacro)

find_dependency(Vulkan)

if (ANDROID)
    # TODO
elseif (WIN32)
    # just use native windowing
elseif (APPLE)
    # just use native windowing
else()
    # use Xcb for native windowing
    find_package(PkgConfig)
    pkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/vsgTargets.cmake")
