SET(APPLICATION_NAME vsgdraw)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/
    ${GLFW_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(HEADERS
    vulcan/Instance.h
    vulcan/Surface.h
    vulcan/PhysicalDevice.h
    vulcan/Device.h
    vulcan/Swapchain.h
    vulcan/CmdDraw.h
)

SET(SOURCES
    vsgdraw.cpp
)

ADD_EXECUTABLE(${APPLICATION_NAME} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${APPLICATION_NAME} vsg  ${Vulkan_LIBRARY} ${GLFW_LIBRARY})
