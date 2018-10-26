cmake_minimum_required(VERSION 3.5)

project(VulkanSceneGraphPrototype)

set(VSG_MAJOR_VERSION 0)
set(VSG_MINOR_VERSION 0)
set(VSG_PATCH_VERSION 0)
set(VSG_SOVERSION 0)

# create the version header
set(VSG_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/vsg/core/Version.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/vsg/core/Version.h.in" "${VSG_VERSION_HEADER}")


# find the various depdendencies
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMakeModules/")

set(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
set(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    # set up local bin directory to place all binaries
    make_directory(${OUTPUT_BINDIR})
    make_directory(${OUTPUT_LIBDIR})
else(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    # set up local bin directory to place all binaries
    make_directory(${OUTPUT_LIBDIR})
endif(WIN32)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})

# set compilation to use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Change the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(VSG_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wmaybe-uninitialized -Wshadow -Wunused -Wno-misleading-indentation -Wextra -pedantic)
endif()

set(VSG_WARNING_FLAGS ${VSG_WARNING_FLAGS} CACHE STRING "Compiler flags to use." FORCE)


foreach (flag ${VSG_WARNING_FLAGS})
    if(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endif()
endforeach ()


find_package(GLFW)
find_package(Vulkan)

find_package(Threads)

find_library(DL_LIBRARY dl)
if(NOT DL_LIBRARY)
    set(DL_LIBRARY "") # change from NOTFOUND to empty when passed to linker
endif()


add_custom_target(clobber
    COMMAND git clean -d -f -x
)

add_custom_target(build_all_h
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/build_all_h.cmake
)

find_program(CPPCHECK cppcheck)
if (CPPCHECK)
    file(RELATIVE_PATH PATH_TO_SOURCE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
    if (PATH_TO_SOURCE)
        set(PATH_TO_SOURCE "${PATH_TO_SOURCE}/")
    endif()

    set(CPPCHECK_SUPPRESION_LIST_FILE "${CMAKE_SOURCE_DIR}/CMakeModules/cppcheck-suppression-list.txt")
    set(CPPCHECK_SUPPRESION_LIST "--suppressions-list=${CPPCHECK_SUPPRESION_LIST_FILE}")
    set(CPPCHECK_EXTRA_OPTIONS "" CACHE STRING "additional commndline options to use when invoking cppcheck")
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK} --quiet --enable=style ${CPPCHECK_EXTRA_OPTIONS} ${CPPCHECK_SUPPRESION_LIST} ${PATH_TO_SOURCE}src/vsg/*/*.cpp -I ${PATH_TO_SOURCE}include/
    )
endif()

# src directory contains all the example applications etc.
add_subdirectory(src/vsg)
