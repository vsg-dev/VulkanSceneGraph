if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")

    file(READ "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" files)
    string(REGEX REPLACE "\n" ";" files "${files}")

    foreach(file ${files})
        if(EXISTS "${file}")
            message(STATUS "Uninstalling \"${file}\"")
            file(REMOVE_RECURSE ${file})
        endif()
    endforeach()

else()
    message(FATAL_ERROR "Cannot find install manifest: \"${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\"")
endif()
