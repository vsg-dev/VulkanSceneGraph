cmake_minimum_required(VERSION 3.5)

project(VulkanPlayground)

SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
IF(WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
ELSE(WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
ENDIF(WIN32)

# set up local bin directory to place all binaries
make_directory(${OUTPUT_BINDIR})
make_directory(${OUTPUT_LIBDIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})

# find the various depdendencies
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMakeModules/")

# set compilation to use C++11
set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD 14)

# Change the default build type to Release
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

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

find_package(Threads)

find_library(DL_LIBRARY dl)
IF(NOT DL_LIBRARY)
    SET(DL_LIBRARY "") # change from NOTFOUND to empty when passed to linker
ENDIF()



# src directory contains all the example applications etc.
add_subdirectory(src)
add_subdirectory(applications)