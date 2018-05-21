cmake_minimum_required(VERSION 3.5)

project(VulkanPlayground)


set(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
make_directory(${OUTPUT_BINDIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})

# Find OpenSceneGraph packages for reference
find_package(OpenThreads)
find_package(osg)
find_package(osgDB)
find_package(osgUtil)
find_package(osgGA)
find_package(osgViewer)

# src directory contains all the example applications etc.
add_subdirectory(src)