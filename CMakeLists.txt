cmake_minimum_required(VERSION 3.5)

project(VulkanPlayground)

# set up local bin directory to place all binaries
set(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
make_directory(${OUTPUT_BINDIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})


# find the various depdendencies
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMakeModules/")

# Find OpenSceneGraph packages for reference
find_package(OpenThreads)
find_package(osg)
find_package(osgDB)
find_package(osgUtil)
find_package(osgGA)
find_package(osgViewer)

find_package(GLM)
find_package(GLFW)
find_package(Vulkan)

# src directory contains all the example applications etc.
add_subdirectory(src)