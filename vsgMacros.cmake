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
macro(add_option_maintainer)
    set(options)
    set(oneValueArgs PREFIX RCLEVEL)
    set(multiValueArgs)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    option(MAINTAINER "Enable maintainer build methods, such as making git branches and tags." OFF)
    if(MAINTAINER)

        #
        # Provide target for tagging a release
        #
        set(VSG_BRANCH ${ARGS_PREFIX}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

        set(GITCOMMAND git)
        set(ECHO ${CMAKE_COMMAND} -E echo)

        if(ARGS_RCLEVEL EQUAL 0)
            set(RELEASE_NAME ${ARGS_PREFIX}-${PROJECT_VERSION})
        else()
            set(RELEASE_NAME ${ARGS_PREFIX}-${PROJECT_VERSION}-rc${ARGS_RCLEVEL})
        endif()

        set(RELEASE_MESSAGE "Release ${RELEASE_NAME}")
        set(BRANCH_MESSAGE "Branch ${VSG_BRANCH}")

        add_custom_target(tag-test
            COMMAND ${ECHO} ${GITCOMMAND} tag -a ${RELEASE_NAME} -m ${RELEASE_MESSAGE}
            COMMAND ${ECHO} ${GITCOMMAND} push origin ${RELEASE_NAME}
        )

        add_custom_target(tag-run
            COMMAND ${GITCOMMAND} tag -a ${RELEASE_NAME} -m ${RELEASE_MESSAGE}
            COMMAND ${GITCOMMAND} push origin ${RELEASE_NAME}
        )

        add_custom_target(branch-test
            COMMAND ${ECHO} ${GITCOMMAND} branch ${VSG_BRANCH}
            COMMAND ${ECHO} ${GITCOMMAND} push origin ${VSG_BRANCH}
        )

        add_custom_target(branch-run
            COMMAND ${GITCOMMAND} branch ${VSG_BRANCH}
            COMMAND ${GITCOMMAND} push origin ${VSG_BRANCH}
        )

    endif()
endmacro()
