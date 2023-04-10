#
# macros provided by the vsg library
#

find_package(Git QUIET)

# give hint for cmake developers
if(NOT _vsg_macros_included)
    message(STATUS "Reading 'vsg_...' macros from ${CMAKE_CURRENT_LIST_DIR}/vsgMacros.cmake - look there for documentation")
    set(_vsg_macros_included 1)
endif()

#
# setup build related variables
#
macro(vsg_setup_build_vars)
    set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
    set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
    set(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
    set(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

    # Change the default build type to Release
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif(NOT CMAKE_BUILD_TYPE)


    if(CMAKE_COMPILER_IS_GNUCXX)
        set(VSG_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wmaybe-uninitialized -Wshadow -Wunused -Wno-misleading-indentation -Wextra CACHE STRING "Compile flags to use.")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(VSG_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wshadow -Wunused -Wextra CACHE STRING "Compile flags to use.")
    else()
        set(VSG_WARNING_FLAGS CACHE STRING "Compile flags to use.")
    endif()

    add_compile_options(${VSG_WARNING_FLAGS})

    # set upper case <PROJECT>_VERSION_... variables
    string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)
    set(${UPPER_PROJECT_NAME}_VERSION ${PROJECT_VERSION})
    set(${UPPER_PROJECT_NAME}_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(${UPPER_PROJECT_NAME}_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(${UPPER_PROJECT_NAME}_VERSION_PATCH ${PROJECT_VERSION_PATCH})
endmacro()

#
# setup directory related variables
#
macro(vsg_setup_dir_vars)
    set(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
    set(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})

    include(GNUInstallDirs)

    if(WIN32)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
        # set up local bin directory to place all binaries
        make_directory(${OUTPUT_BINDIR})
        make_directory(${OUTPUT_LIBDIR})
        set(VSG_INSTALL_TARGETS_DEFAULT_FLAGS
            EXPORT ${PROJECT_NAME}Targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    else()
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
        # set up local bin directory to place all binaries
        make_directory(${OUTPUT_LIBDIR})
        set(VSG_INSTALL_TARGETS_DEFAULT_FLAGS
            EXPORT ${PROJECT_NAME}Targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()

    # provide backwards compatibility to VulkanSceneGraph-1.0.0
    set(INSTALL_TARGETS_DEFAULT_FLAGS ${VSG_INSTALL_TARGETS_DEFAULT_FLAGS})
endmacro()

#
# create and install cmake support files
#
# available arguments:
#
#    CONFIG_TEMPLATE <file>   cmake template file for generating <prefix>Config.cmake file
#    [PREFIX <prefix>]        prefix for generating file names (optional)
#                             If not specified, ${PROJECT_NAME} is used
#    [VERSION <version>]      version used for <prefix>ConfigVersion.cmake (optional)
#                             If not specified, ${PROJECT_VERSION} is used
#
# The files maintained by this macro are
#
#    <prefix>Config.cmake
#    <prefix>ConfigVersion.cmake
#    <prefix>Targets*.cmake
#
macro(vsg_add_cmake_support_files)
    set(options)
    set(oneValueArgs CONFIG_TEMPLATE PREFIX)
    set(multiValueArgs)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARGS_PREFIX)
        set(ARGS_PREFIX ${PROJECT_NAME})
    endif()
    if(NOT ARGS_VERSION)
        set(ARGS_VERSION ${PROJECT_VERSION})
    endif()
    set(CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_PREFIX}Config.cmake)
    set(CONFIG_VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_PREFIX}ConfigVersion.cmake)

    if(NOT ARGS_CONFIG_TEMPLATE)
        message(FATAL_ERROR "no template for generating <prefix>Config.cmake provided - use argument CONFIG_TEMPLATE <file>")
    endif()
    configure_file(${ARGS_CONFIG_TEMPLATE} ${CONFIG_FILE} @ONLY)

    install(EXPORT ${ARGS_PREFIX}Targets
        FILE ${ARGS_PREFIX}Targets.cmake
        NAMESPACE ${ARGS_PREFIX}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ARGS_PREFIX}
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CONFIG_VERSION_FILE}
        COMPATIBILITY SameMajorVersion
        VERSION ${ARGS_VERSION}
    )

    install(
        FILES
            ${CONFIG_FILE}
            ${CONFIG_VERSION_FILE}
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/${ARGS_PREFIX}
    )
endmacro()

#
# add feature summary
#
macro(vsg_add_feature_summary)
    include(FeatureSummary)
    feature_summary(WHAT ALL)
endmacro()

#
# add 'MAINTAINER' option
#
# available arguments:
#
#    PREFIX    prefix for branch and tag name
#    RCLEVEL   release candidate level
#
# added cmake targets:
#
#    tag-run      create a tag in the git repository with name <prefix>-<major>.<minor>.<patch>
#    branch-run   create a branch in the git repository with name <prefix>-<major>.<minor>
#    tag-test     show the command to create a tag in the git repository
#    branch-test  show the command to create a branch in the git repository
#
macro(vsg_add_option_maintainer)

    if(Git_FOUND)
        set(options)
        set(oneValueArgs PREFIX RCLEVEL)
        set(multiValueArgs)
        cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        option(MAINTAINER "Enable maintainer build methods, such as making git branches and tags." OFF)
        if(MAINTAINER)

            #
            # Provide target for tagging a release
            #
            set(VSG_BRANCH ${ARGS_PREFIX}${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

            set(GITCOMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR})
            set(ECHO ${CMAKE_COMMAND} -E echo)
            set(REMOTE origin)

            if(ARGS_RCLEVEL EQUAL 0)
                set(RELEASE_NAME ${ARGS_PREFIX}${PROJECT_VERSION})
            else()
                set(RELEASE_NAME ${ARGS_PREFIX}${PROJECT_VERSION}-rc${ARGS_RCLEVEL})
            endif()

            set(RELEASE_MESSAGE "Release ${RELEASE_NAME}")
            set(BRANCH_MESSAGE "Branch ${VSG_BRANCH}")

            add_custom_target(tag-test
                COMMAND ${ECHO} ${GITCOMMAND} tag -a ${RELEASE_NAME} -m \"${RELEASE_MESSAGE}\"
                COMMAND ${ECHO} ${GITCOMMAND} push ${REMOTE} ${RELEASE_NAME}
            )

            add_custom_target(tag-run
                COMMAND ${GITCOMMAND} tag -a ${RELEASE_NAME} -m "${RELEASE_MESSAGE}"
                COMMAND ${GITCOMMAND} push ${REMOTE} ${RELEASE_NAME}
            )

            add_custom_target(branch-test
                COMMAND ${ECHO} ${GITCOMMAND} branch ${VSG_BRANCH}
                COMMAND ${ECHO} ${GITCOMMAND} push ${REMOTE} ${VSG_BRANCH}
            )

            add_custom_target(branch-run
                COMMAND ${GITCOMMAND} branch ${VSG_BRANCH}
                COMMAND ${GITCOMMAND} push ${REMOTE} ${VSG_BRANCH}
            )

        endif()
    endif()
endmacro()

#
# add 'clang-format' build target to enforce a standard code style guide.
#
# available arguments:
#
#    FILES      list with file names or file name pattern
#    EXCLUDES   list with file names to exclude from the list
#               given by the FILES argument
#
# To support cmake projects with submodules, it is necessary to use
# PROJECT_SOURCE_DIR as prefix for file names.
#
# If vsg is used in a cmake project with submodules, additional targets
# named 'clang-format-${PROJECT}' are created for the corresponding
# project and added as a dependency to the parent target 'clang-format'.
#
macro(vsg_add_target_clang_format)
    set(options)
    set(oneValueArgs )
    set(multiValueArgs FILES EXCLUDE)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    find_program(CLANGFORMAT clang-format)
    if (CLANGFORMAT)
        file(GLOB FILES_TO_FORMAT
            ${ARGS_FILES}
        )
        foreach(EXCLUDE ${ARGS_EXCLUDES})
            list(REMOVE_ITEM FILES_TO_FORMAT ${EXCLUDE})
        endforeach()
        if (NOT TARGET clang-format)
            add_custom_target(clang-format)
        endif()
        add_custom_target(clang-format-${PROJECT_NAME}
            COMMAND ${CLANGFORMAT} -i ${FILES_TO_FORMAT}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Automated code format using clang-format"
        )
        set_target_properties(clang-format-${PROJECT_NAME} PROPERTIES FOLDER "${PROJECT_NAME} Folder")
        add_dependencies(clang-format clang-format-${PROJECT_NAME})
    endif()
endmacro()

#
# add 'clobber' build target to clear all the non git registered files/directories
#
# If vsg is used in a cmake project with submodules and out-of-source builds, additional
# targets named 'clobber-${PROJECT}' are created for the corresponding
# project and added as a dependency to the parent target 'clobber'
#
macro(vsg_add_target_clobber)
    if(Git_FOUND)
        # in source builds does not support dependencies here
        # see https://github.com/vsg-dev/VulkanSceneGraph/pull/566#issuecomment-1312496507
        if (PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
            add_custom_target(clobber
                COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} clean -d -f -x
            )
        else()
            if (NOT TARGET clobber)
                add_custom_target(clobber)
            endif()
            add_custom_target(clobber-${PROJECT_NAME}
                COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} clean -d -f -x
            )
            set_target_properties(clobber-${PROJECT_NAME} PROPERTIES FOLDER "${PROJECT_NAME} Folder")
            add_dependencies(clobber clobber-${PROJECT_NAME})
        endif()
    endif()
endmacro()

#
# add 'cppcheck' build target to provide static analysis of codebase
#
# available arguments:
#
#    FILES             list with file names or file name pattern
#    SUPPRESSIONS_LIST filename for list with suppressions (optional)
#
# used global cmake variables:
#
#    CPPCHECK_EXTRA_OPTIONS - add extra options to cppcheck command line
#
# To support cmake projects with submodules, it is necessary to use
# PROJECT_SOURCE_DIR as prefix for file names.

# If vsg is used in a cmake project with submodules, additional targets
# named 'cppcheck-${PROJECT}' are created for the corresponding
# project and added as a dependency to the parent target 'cppcheck'.
#
macro(vsg_add_target_cppcheck)
    set(options)
    set(oneValueArgs SUPPRESSIONS_LIST)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    find_program(CPPCHECK cppcheck)
    if (CPPCHECK)
        file(RELATIVE_PATH PATH_TO_SOURCE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
        if (PATH_TO_SOURCE)
            set(PATH_TO_SOURCE "${PATH_TO_SOURCE}/")
        endif()

        include(ProcessorCount)
        ProcessorCount(CPU_CORES)

        if(ARGS_SUPPRESSIONS_LIST)
            set(SUPPRESSION_LIST "--suppressions-list=${ARGS_SUPPRESSIONS_LIST}")
        endif()
        set(CPPCHECK_EXTRA_OPTIONS "" CACHE STRING "additional commandline options to use when invoking cppcheck")
        if (NOT TARGET cppcheck)
            add_custom_target(cppcheck)
        endif()
        add_custom_target(cppcheck-${PROJECT_NAME}
            COMMAND ${CPPCHECK} -j ${CPU_CORES} --quiet --enable=style --language=c++
                ${CPPCHECK_EXTRA_OPTIONS}
                ${SUPPRESSION_LIST}
                ${ARGS_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Static code analysis using cppcheck"
        )
        set_target_properties(cppcheck PROPERTIES FOLDER "${PROJECT_NAME} Folder")
        add_dependencies(cppcheck cppcheck-${PROJECT_NAME})
    endif()
endmacro()

#
# add 'docs' build target to provide API documentation generated by doxygen
#
# available arguments:
#
#    FILES      list with file or directory names
#
# To support cmake projects with submodules, it is necessary to use
# PROJECT_SOURCE_DIR as prefix for file names.
#
# If vsg is used in a cmake project with submodules, additional targets
# named 'docs-${PROJECT}' are created for the corresponding
# project and added as a dependency to the parent target 'docs'.
#
macro(vsg_add_target_docs)
    set(options)
    set(oneValueArgs )
    set(multiValueArgs FILES)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # create doxygen build target
    find_package(Doxygen QUIET)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_MAN NO)
        if (NOT TARGET docs)
            add_custom_target(docs)
        endif()
        doxygen_add_docs(
            docs-${PROJECT_NAME}
            ${ARGS_FILES}
            COMMENT "Use doxygen to Generate html documentation"
        )
        set_target_properties(docs PROPERTIES FOLDER "${PROJECT_NAME} Folder")
        add_dependencies(docs docs-${PROJECT_NAME})
    endif()
endmacro()

#
# add 'uninstall' build target
#
# If vsg is used in a cmake project with submodules, additional targets
# named 'uninstall-${PROJECT}' are created for the corresponding
# project and added as a dependency to the parent target 'uninstall'.
#
macro(vsg_add_target_uninstall)
    # we are running inside VulkanSceneGraph
    if (PROJECT_NAME STREQUAL "vsg")
        set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    elseif(vsg_DIR)
        set(DIR ${vsg_DIR})
    else()
        set(DIR ${CMAKE_CURRENT_LIST_DIR})
    endif()
    if (NOT TARGET uninstall)
        add_custom_target(uninstall)
    endif()
    add_custom_target(uninstall-${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -P ${DIR}/uninstall.cmake
    )
    set_target_properties(uninstall-${PROJECT_NAME} PROPERTIES FOLDER "${PROJECT_NAME} Folder")
    add_dependencies(uninstall uninstall-${PROJECT_NAME})
endmacro()

#
# check minimum header version of Vulkan
#
# available arguments:
#
#    <min_version>      minimum required version e.g. 1.2.194
#
macro(vsg_check_min_vulkan_header_version _min_version)
    if (Vulkan_FOUND)
        set(Vulkan_VERSION "")
        set(VULKAN_CORE_H ${Vulkan_INCLUDE_DIR}/vulkan/vulkan_core.h)
        if(EXISTS ${VULKAN_CORE_H})
          file(STRINGS  ${VULKAN_CORE_H} VulkanHeaderVersionLine REGEX "^#define VK_HEADER_VERSION ")
          string(REGEX MATCHALL "[0-9]+" VulkanHeaderVersion "${VulkanHeaderVersionLine}")
          file(STRINGS  ${VULKAN_CORE_H} VulkanHeaderVersionLine2 REGEX "^#define VK_HEADER_VERSION_COMPLETE ")
          string(REGEX MATCHALL "[0-9]+" VulkanHeaderVersion2 "${VulkanHeaderVersionLine2}")
          list(LENGTH VulkanHeaderVersion2 _len)
          #  versions >= 1.2.175 adds an additional numbers in front of e.g. '0, 1, 2' instead of '1, 2'
          if(_len EQUAL 3)
              list(REMOVE_AT VulkanHeaderVersion2 0)
          endif()
          list(APPEND VulkanHeaderVersion2 ${VulkanHeaderVersion})

          set(Vulkan_VERSION "")
          foreach(value ${VulkanHeaderVersion2})
            if (Vulkan_VERSION STREQUAL "")
                set(Vulkan_VERSION "${value}")
            else()
                set(Vulkan_VERSION "${Vulkan_VERSION}.${value}")
            endif()
          endforeach()

        endif()
        if(Vulkan_VERSION VERSION_LESS ${_min_version})
            message(FATAL_ERROR "Could NOT find Vulkan: Found unsuitable version \"${Vulkan_VERSION}\", but required is at least \"${_min_version}\" (found ${Vulkan_LIBRARY})")
        else()
            message(STATUS "Found Vulkan: ${Vulkan_LIBRARY} (found suitable version \"${Vulkan_VERSION}\", minimum required is \"${_min_version}\")")
        endif()
    else()
        message(FATAL_ERROR "Could NOT find Vulkan: version check not possible - use 'find_package(Vulkan REQUIRED)' to fix this")
    endif()
endmacro()

#
# add options for vsg and all packages depending on vsg
#
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
