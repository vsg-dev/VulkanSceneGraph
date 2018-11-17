include(CMakeFindDependencyMacro)

find_dependency(Vulkan)

if(NOT WIN32)
    find_dependency(glfw3)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/vsgTargets.cmake")
