INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
)

SET(HEADERS

    core/observer_ptr.h
    core/ref_ptr.h
    core/Auxiliary.h
    core/Object.h
    core/Value.h
    core/Visitor.h

    maths/vec2.h
    maths/vec3.h
    maths/vec4.h
    maths/mat4.h

    nodes/Group.h
    nodes/Node.h
    nodes/QuadGroup.h
    nodes/LOD.h

    vk/Instance.h
    vk/PhysicalDevice.h
    vk/Device.h
    vk/Surface.h
    vk/ImageView.h
    vk/Swapchain.h
    vk/CmdDraw.h
    vk/Pipeline.h
    vk/PipelineLayouyt.h

    utils/CommandLine.h
)

SET(SOURCES

    core/Auxiliary.cpp
    core/Object.cpp
    core/Visitor.cpp

    nodes/Group.cpp
    nodes/Node.cpp

    vk/PhysicalDevice.cpp
    vk/Device.cpp
    vk/Surface.cpp
    vk/ImageView.cpp
    vk/Instance.cpp
    vk/Swapchain.cpp
    vk/ShaderModule.cpp
    vk/RenderPass.cpp
    vk/Pipeline.cpp
    vk/PipelineLayout.cpp
)

ADD_LIBRARY(vsg SHARED ${HEADER_FILES} ${SOURCES})

TARGET_LINK_LIBRARIES(vsg  ${Vulkan_LIBRARY})
