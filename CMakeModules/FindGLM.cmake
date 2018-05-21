# FindGLM,cmake sourced from the GL_vs_VK project, (C) Copyright (c) 2017 Damian Dyńdo
#
#
# GLM_FOUND
# GLM_INCLUDE_DIRS
#

include(FindPackageHandleStandardArgs)

if (WIN32)
    find_path( GLM_INCLUDE_DIRS
        NAMES
            glm/glm.hpp
        PATHS
            ${PROJECT_SOURCE_DIR}/third_party/glm/
            ${PROJECT_SOURCE_DIR}/../third_party/glm/
            ${GLM_LOCATION}/
            $ENV{GLM_LOCATION}/include
            $ENV{PROGRAMFILES}/glm/
            $ENV{PROGRAMFILES}/glm/include
            ${GLM_LOCATION}
            $ENV{GLM_LOCATION}
            DOC "The directory where glm/glm.hpp resides" )
endif ()

if (${CMAKE_HOST_UNIX})
    find_path( GLM_INCLUDE_DIRS
        NAMES
            glm/glm.hpp
        PATHS
            ${PROJECT_SOURCE_DIR}/third_party/glm/
            ${PROJECT_SOURCE_DIR}/../third_party/glm/
            ${GLM_LOCATION}/include
            $ENV{GLM_LOCATION}/include
            /usr/include
            /usr/local/include
            /sw/include
            /opt/local/include
            NO_DEFAULT_PATH
            DOC "The directory where glm/glm.hpp resides"
    )
endif ()

find_package_handle_standard_args(GLM DEFAULT_MSG
    GLM_INCLUDE_DIRS
)

# mark_as_advanced( GLM_FOUND )
