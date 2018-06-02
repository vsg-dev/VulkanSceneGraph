# FindGLFW.cmake
#
# Based on FindGLFW,cmake sourced from the GL_vs_VK project, (C) Copyright (c) 2017 Damian Dyńdo
#
# This module is taken from CMake original find-modules and adapted for the needs of this project
# Below notice is from original file:
# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindGLFW
# ----------
#
# Try to find GLFW
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``GLFW::GLFW``, if
# GLFW has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables::
#
#   GLFW_FOUND           - True if GLFW was found
#   GLFW_INCLUDE_DIRS    - include directories for GLFW and third-party GLFW-Hpp
#   GLFW_LIBRARIES       - link against this library to use GLFW
#
# The module will also define the following cache variables::
#
#   GLFW_INCLUDE_DIR     - the GLFW include directory
#   GLFW_Hpp_INCLUDE_DIR - the GLFW-Hpp include directory
#   GLFW_LIBRARY         - the path to the GLFW library
#

# GLFW header file
find_path(GLFW_INCLUDE_DIR
  NAMES GLFW/glfw3.h
  PATHS
    "$ENV{GLFW_SDK}/include"
  )


# GLFW library
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(GLFW_LIBRARY
      NAMES glfw3  glfw
      PATHS
        "$ENV{GLFW_SDK}/Lib"
        "$ENV{GLFW_SDK}/Bin"
        )
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    find_library(GLFW_LIBRARY
      NAMES glfw3-1  glfw
      PATHS
        "$ENV{GLFW_SDK}/Lib32"
        "$ENV{GLFW_SDK}/Bin32"
        NO_SYSTEM_ENVIRONMENT_PATH
        )
  endif()
else()
    find_library(GLFW_LIBRARY
      NAMES glfw3 glfw
      PATHS
        "$ENV{GLFW_SDK}/lib")
endif()

set(GLFW_LIBRARIES ${GLFW_LIBRARY})
set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW
  DEFAULT_MSG
  GLFW_LIBRARY GLFW_INCLUDE_DIR)

# mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)

if(GLFW_FOUND AND NOT TARGET GLFW::GLFW)
  add_library(GLFW::GLFW UNKNOWN IMPORTED)
  set_target_properties(GLFW::GLFW PROPERTIES
    IMPORTED_LOCATION "${GLFW_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIRS}")
endif()
