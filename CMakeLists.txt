cmake_minimum_required(VERSION 3.7)

project(VSG
    VERSION 0.0.0
    DESCRIPTION "Vulkan/VkSceneGraph Prototype library"
    LANGUAGES CXX
)
set(VSG_SOVERSION 0)

# create the version header
set(VSG_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/vsg/core/Version.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/vsg/core/Version.h.in" "${VSG_VERSION_HEADER}")

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

# Change the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(VSG_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wmaybe-uninitialized -Wshadow -Wunused -Wno-misleading-indentation -Wextra) # removed -pedantic due to macro(); warnings
endif()
set(VSG_WARNING_FLAGS ${VSG_WARNING_FLAGS} CACHE STRING "Compiler flags to use." FORCE)
add_compile_options(${VSG_WARNING_FLAGS})


find_package(Vulkan)

if (ANDROID)
    find_library(AndroidLib android)
    if(CMAKE_SYSTEM_VERSION GREATER 24)
        find_library(AndroidNativeWindowLib nativewindow)
    endif()
elseif (WIN32)
    # just use native windowing
elseif (APPLE)
    find_package(glfw3)
else()
    find_package(glfw3)

    find_package(PkgConfig)
    pkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)
    option(USE_GLFW "Use GLFW for Windowing" OFF)
endif()



# create doxygen build target
find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)

    doxygen_add_docs(
        docs
        ${PROJECT_SOURCE_DIR}/include/vsg
        COMMENT "Use doxygen to Generate html documentaion"
    )
endif()

# add clobber build target to clear all the non git registered files/directories
add_custom_target(clobber
    COMMAND git clean -d -f -x
)

# automatically buil_all_h build target to generate the include/vsg/all.h from the headers in the include/vsg/* directories
add_custom_target(build_all_h
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/build/build_all_h.cmake
)

# automatically buil_all_h build target to generate the include/vsg/all.h from the headers in the include/vsg/* directories
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/build/uninstall.cmake
)

# add cppcheck build target to provide static analysis of codebase
find_program(CPPCHECK cppcheck)
if (CPPCHECK)
    file(RELATIVE_PATH PATH_TO_SOURCE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
    if (PATH_TO_SOURCE)
        set(PATH_TO_SOURCE "${PATH_TO_SOURCE}/")
    endif()

    set(CPPCHECK_SUPPRESION_LIST_FILE "${CMAKE_SOURCE_DIR}/build/cppcheck-suppression-list.txt")
    set(CPPCHECK_SUPPRESION_LIST "--suppressions-list=${CPPCHECK_SUPPRESION_LIST_FILE}")
    set(CPPCHECK_EXTRA_OPTIONS "" CACHE STRING "additional commndline options to use when invoking cppcheck")
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK} --quiet --enable=style --language=c++  ${CPPCHECK_EXTRA_OPTIONS} ${CPPCHECK_SUPPRESION_LIST} ${PATH_TO_SOURCE}include/vsg/*/*.h ${PATH_TO_SOURCE}src/vsg/*/*.cpp -I ${PATH_TO_SOURCE}include/
    )
endif()

# add clang-foramt build target to enforce a standard code style guide.
find_program(CLANGFORMAT clang-format)
if (CLANGFORMAT)
    add_custom_target(clang-format
        COMMAND ${CLANGFORMAT} -i ${PATH_TO_SOURCE}include/vsg/*/*.h ${PATH_TO_SOURCE}src/vsg/*/*.cpp
    )
endif()

#
# src directory contains all the source of the vsg library
#
add_subdirectory(src/vsg)
