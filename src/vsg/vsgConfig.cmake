include(CMakeFindDependencyMacro)

find_dependency(Vulkan)

include("${CMAKE_CURRENT_LIST_DIR}/vsgTargets.cmake")
