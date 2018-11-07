include(CMakeFindDependencyMacro)

find_dependency(Vulkan)
find_dependency(glfw3)

include("${CMAKE_CURRENT_LIST_DIR}/vsgTargets.cmake")
