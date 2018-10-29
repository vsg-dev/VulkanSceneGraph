# FindVSG,cmake sourced from the GL_vs_VK project, (C) Copyright (c) 2017 Damian Dyńdo
#
# This module is taken from CMake original find-modules and adapted for the
# needs of this project:
# Below notice is from original file:
#
# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindVSG
# ----------
#
# Try to find VSG
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``VSG::VSG``, if
# VSG has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables::
#
#   VSG_FOUND           - True if VSG was found
#   VSG_INCLUDE_DIRS    - include directories for VSG and third-party VSG-Hpp
#   VSG_LIBRARIES       - link against this library to use VSG
#
# The module will also define the following cache variables::
#
#   VSG_INCLUDE_DIR     - the VSG include directory
#   VSG_LIBRARY         - the path to the VSG library
#

# VSG header file

find_path(VSG_INCLUDE_DIR
  NAMES vsg/all.h
  PATHS
    ${VSG_DIR}/include
    "$ENV{VSG_DIR}/include"
  )

find_library(VSG_LIBRARY_RELEASE
    NAMES vsg vsgs
    PATHS
    ${VSG_DIR}/lib
    "$ENV{VSG_DIR}/lib"
)

find_library(VSG_LIBRARY_DEBUG
    NAMES vsgd vsgrd
    PATHS
    ${VSG_DIR}/lib
    "$ENV{VSG_DIR}/lib"
)

set(VSG_LIBRARIES ${VSG_LIBRARY})
set(VSG_INCLUDE_DIRS ${VSG_INCLUDE_DIR})

include(SelectLibraryConfigurations)
select_library_configurations(VSG)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VSG REQUIRED_VARS VSG_LIBRARY VSG_INCLUDE_DIR)


if (VSG_FOUND)

    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CmakeTmp/vsg_test.cxx"
    "
    #include <vsg/core/Version.h>
    #include <iostream>
    int main(int, char**)
    {
        std::cout<<vsgGetVersion()<<std::endl;
        return vsgBuildAsSharedLibrary();
    }
    \n"
    )

    try_run(RunResult CompileResult
        "${CMAKE_BINARY_DIR}"
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CmakeTmp/vsg_test.cxx
        CMAKE_FLAGS -DINCLUDE_DIRECTORIES:STRING=${VSG_INCLUDE_DIR}
        LINK_LIBRARIES ${VSG_LIBRARY}
        COMPILE_OUTPUT_VARIABLE CompileOutput
        RUN_OUTPUT_VARIABLE RunOutput
    )

    #message("file " ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CmakeTmp/vsg_test.cxx)
    #message("CompileResult " ${CompileResult})
    #message("RunResult " ${RunResult})
    #message("RunOutput " ${RunOutput})

    if (${CompileResult})
        if (${RunResult} EQUAL 1)
            set(VSG_DEFINITIONS VSG_SHARED_LIBRARY)
        endif()
    else()
        message("Compile Error compiling vsg library type test application, follow is build output of test:\n\n" ${CompileOutput})
    endif()

endif()

# mark_as_advanced(VSG_INCLUDE_DIR VSG_LIBRARY)

if(VSG_FOUND)

    if (NOT TARGET VSG::VSG)

        add_library(VSG::VSG UNKNOWN IMPORTED)

        set_target_properties(VSG::VSG PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${VSG_INCLUDE_DIR}")

        if (VSG_LIBRARY_RELEASE)
            set_property(TARGET VSG::VSG APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(VSG::VSG PROPERTIES IMPORTED_LOCATION_RELEASE "${VSG_LIBRARY_RELEASE}")
        endif()

        if (VSG_LIBRARY_DEBUG)
            set_property(TARGET VSG::VSG APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(VSG::VSG PROPERTIES IMPORTED_LOCATION_DEBUG "${VSG_LIBRARY_DEBUG}")
        endif()

    endif()

endif()
