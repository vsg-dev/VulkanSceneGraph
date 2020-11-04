#.rst:
# Findglslang
# ----------
#
# Try to find glslang in the VulkanSDK
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``glslang::glslang``, if
# glslang has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables::
#
#   glslang_FOUND          - True if glslang was found
#   glslang_INCLUDE_DIRS   - include directories for glslang
#   glslang_LIBRARIES      - link against this library to use glslang
#
# The module will also define two cache variables::
#
#   glslang_INCLUDE_DIR    - the glslang include directory
#   glslang_LIBRARY        - the path to the glslang library
#

if (DEFINED ENV{VULKAN_SDK})
    if(WIN32)
        set(ADDITIONAL_PATHS_INCLUDE "$ENV{VULKAN_SDK}/Include")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ADDITIONAL_PATHS_LIBS
                "$ENV{VULKAN_SDK}/Lib"
                "$ENV{VULKAN_SDK}/Bin"
            )
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(ADDITIONAL_PATHS_LIBS
                "$ENV{VULKAN_SDK}/Lib32"
                "$ENV{VULKAN_SDK}/Bin32"
            )
        endif()
    else()
        set(ADDITIONAL_PATHS_INCLUDE "$ENV{VULKAN_SDK}/include")
        set(ADDITIONAL_PATHS_LIBS "$ENV{VULKAN_SDK}/lib")
    endif()
endif()

find_path(glslang_INCLUDE_DIR
    NAMES glslang/Public/ShaderLang.h
    PATHS ${ADDITIONAL_PATHS_INCLUDE}
)

find_path(spirv_INCLUDE_DIR
    NAMES glslang/SPIRV/GlslangToSpv.h
    PATHS ${ADDITIONAL_PATHS_INCLUDE}
)

find_library(glslang_LIBRARY
    NAMES glslang
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(OSDependent_LIBRARY
    NAMES OSDependent
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(SPIRV_LIBRARY
    NAMES SPIRV
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(OGLCompiler_LIBRARY
    NAMES OGLCompiler
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(HLSL_LIBRARY
    NAMES HLSL
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(MachineIndependent_LIBRARY
    NAMES MachineIndependent
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(GenericCodeGen_LIBRARY
    NAMES GenericCodeGen
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(SPIRV-Tools_LIBRARY
    NAMES SPIRV-Tools
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

find_library(SPIRV-Tools-opt_LIBRARY
    NAMES SPIRV-Tools-opt
    PATHS ${ADDITIONAL_PATHS_LIBS}
)

if(WIN32)

    find_library(glslang_LIBRARY_debug
        NAMES glslangd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(OSDependent_LIBRARY_debug
        NAMES OSDependentd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(SPIRV_LIBRARY_debug
        NAMES SPIRVd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(OGLCompiler_LIBRARY_debug
        NAMES OGLCompilerd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(HLSL_LIBRARY_debug
        NAMES HLSLd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(MachineIndependent_LIBRARY_debug
        NAMES MachineIndependentd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(GenericCodeGen_LIBRARY_debug
        NAMES GenericCodeGend
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(SPIRV-Tools_LIBRARY_debug
        NAMES SPIRV-Toolsd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

    find_library(SPIRV-Tools-opt_LIBRARY_debug
        NAMES SPIRV-Tools-optd
        PATHS ${ADDITIONAL_PATHS_LIBS}
    )

endif()


mark_as_advanced(glslang_INCLUDE_DIR glslang_LIBRARY)

if(glslang_LIBRARY AND glslang_INCLUDE_DIR)
    set(glslang_FOUND "YES")
    message(STATUS "Found glslang: ${glslang_LIBRARY}")
else()
    set(glslang_FOUND "NO")
    message(STATUS "Failed to find glslang")
endif()

if(glslang_FOUND AND NOT TARGET glslang::glslang)
    set(glslang_INCLUDE_DIRS ${glslang_INCLUDE_DIR})

    add_library(glslang::glslang UNKNOWN IMPORTED)
    set_target_properties(glslang::glslang PROPERTIES IMPORTED_LOCATION "${glslang_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${glslang_INCLUDE_DIRS}")

    add_library(glslang::OSDependent UNKNOWN IMPORTED)
    set_target_properties(glslang::OSDependent PROPERTIES IMPORTED_LOCATION "${OSDependent_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${OSDependent_INCLUDE_DIRS}")

    add_library(glslang::SPIRV UNKNOWN IMPORTED)
    set_target_properties(glslang::SPIRV PROPERTIES IMPORTED_LOCATION "${SPIRV_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${spirv_INCLUDE_DIR}")

    add_library(glslang::OGLCompiler UNKNOWN IMPORTED)
    set_target_properties(glslang::OGLCompiler PROPERTIES IMPORTED_LOCATION "${OGLCompiler_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${glslang_INCLUDE_DIRS}")

    add_library(glslang::HLSL UNKNOWN IMPORTED)
    set_target_properties(glslang::HLSL PROPERTIES IMPORTED_LOCATION "${HLSL_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${glslang_INCLUDE_DIRS}")

    if (SPIRV-Tools_LIBRARY)
        add_library(glslang::SPIRV-Tools UNKNOWN IMPORTED)
        set_target_properties(glslang::SPIRV-Tools PROPERTIES IMPORTED_LOCATION "${SPIRV-Tools_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${SPIRV-Tools__INCLUDE_DIR}")
    endif()

    if (SPIRV-Tools-opt_LIBRARY)
        add_library(glslang::SPIRV-Tools-opt UNKNOWN IMPORTED)
        set_target_properties(glslang::SPIRV-Tools-opt PROPERTIES IMPORTED_LOCATION "${SPIRV-Tools-opt_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${SPIRV-Tools-opt_INCLUDE_DIR}")
    endif()

    if (MachineIndependent_LIBRARY)
        add_library(glslang::MachineIndependent UNKNOWN IMPORTED)
        set_target_properties(glslang::MachineIndependent PROPERTIES IMPORTED_LOCATION "${MachineIndependent_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${MachineIndependent_INCLUDE_DIRS}")
    endif()

    if (GenericCodeGen_LIBRARY)
        add_library(glslang::GenericCodeGen UNKNOWN IMPORTED)
        set_target_properties(glslang::GenericCodeGen PROPERTIES IMPORTED_LOCATION "${GenericCodeGen_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${GenericCodeGen_INCLUDE_DIRS}")
    endif()

    if(WIN32)
        if (glslang_LIBRARY_debug)
            set_target_properties(glslang::glslang PROPERTIES IMPORTED_LOCATION_DEBUG "${glslang_LIBRARY_debug}")
        endif()
        if (OSDependent_LIBRARY_debug)
            set_target_properties(glslang::OSDependent PROPERTIES IMPORTED_LOCATION_DEBUG "${OSDependent_LIBRARY_debug}")
        endif()
        if (SPIRV_LIBRARY_debug)
            set_target_properties(glslang::SPIRV PROPERTIES IMPORTED_LOCATION_DEBUG "${SPIRV_LIBRARY_debug}")
        endif()
        if (OGLCompiler_LIBRARY_debug)
            set_target_properties(glslang::OGLCompiler PROPERTIES IMPORTED_LOCATION_DEBUG "${OGLCompiler_LIBRARY_debug}")
        endif()
        if (HLSL_LIBRARY_debug)
            set_target_properties(glslang::HLSL PROPERTIES IMPORTED_LOCATION_DEBUG "${HLSL_LIBRARY_debug}")
        endif()
        if (MachineIndependent_LIBRARY_debug)
            set_target_properties(glslang::MachineIndependent PROPERTIES IMPORTED_LOCATION_DEBUG "${MachineIndependent_LIBRARY_debug}")
        endif()
        if (GenericCodeGen_LIBRARY_debug)
            set_target_properties(glslang::GenericCodeGen PROPERTIES IMPORTED_LOCATION_DEBUG "${GenericCodeGen_LIBRARY_debug}")
        endif()
        if (SPIRV-Tools_LIBRARY_debug)
            set_target_properties(glslang::SPIRV-Tools PROPERTIES IMPORTED_LOCATION_DEBUG "${SPIRV-Tools_LIBRARY_debug}")
        endif()
        if (SPIRV-Tools-opt_LIBRARY_debug)
            set_target_properties(glslang::SPIRV-Tools-opt PROPERTIES IMPORTED_LOCATION_DEBUG "${SPIRV-Tools-opt_LIBRARY_debug}")
        endif()
    endif()


    if (MachineIndependent_LIBRARY AND GenericCodeGen_LIBRARY)
        # VULKAN_SDK and associated glslang 1.2.147.1 onwards
        set(glslang_LIBRARIES
            glslang::MachineIndependent
            glslang::OSDependent
            glslang::OGLCompiler
            glslang::SPIRV
            glslang::GenericCodeGen
        )
    else()
        # VULKAN_SDK and associated glslang before 1.2.147.1
        set(glslang_LIBRARIES
            glslang::glslang
            glslang::OSDependent
            glslang::OGLCompiler
            glslang::SPIRV
            glslang::HLSL
        )
    endif()

    if (SPIRV-Tools-opt_LIBRARY)
        list(APPEND glslang_LIBRARIES glslang::SPIRV-Tools-opt)
    endif()
    if (SPIRV-Tools_LIBRARY)
        list(APPEND glslang_LIBRARIES glslang::SPIRV-Tools)
    endif()

endif()
