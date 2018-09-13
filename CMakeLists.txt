cmake_minimum_required(VERSION 3.5)

project(VulkanSceneGraphPrototype)

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

if(CMAKE_COMPILER_IS_GNUCXX)
    set(VSG_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wmaybe-uninitialized -Wshadow -Wunused -Wno-misleading-indentation -Wextra -pedantic)
endif()

SET(VSG_WARNING_FLAGS ${VSG_WARNING_FLAGS} CACHE STRING "Compiler flags to use." FORCE)


foreach (flag ${VSG_WARNING_FLAGS})
    if(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endif()
endforeach ()

# Dynamic vs Static Linking
OPTION(VSG_SHARED_LIBRARY "Set to ON to build VulkanPlayground for dynamic linking.  Use OFF for static." ON)


find_package(GLFW)
find_package(Vulkan)

find_package(Threads)

find_library(DL_LIBRARY dl)
IF(NOT DL_LIBRARY)
    SET(DL_LIBRARY "") # change from NOTFOUND to empty when passed to linker
ENDIF()


# src directory contains all the example applications etc.
add_subdirectory(src)
