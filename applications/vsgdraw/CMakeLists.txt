SET(APPLICATION_NAME vsgdraw)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/
    ${GLFW_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(HEADERS
    vk/Instance.h
    vk/Surface.h
    vk/PhysicalDevice.h
    vk/Device.h
    vk/ImageView.h
    vk/Swapchain.h
    vk/CmdDraw.h
)

SET(SOURCES
    vsgdraw.cpp

    Instance.cpp
    Swapchain.cpp
)

ADD_EXECUTABLE(${APPLICATION_NAME} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${APPLICATION_NAME} vsg  ${Vulkan_LIBRARY} ${GLFW_LIBRARY})
