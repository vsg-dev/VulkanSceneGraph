INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
)

SET(HEADERS

    all.h

    core.h
    core/Auxiliary.h
    core/Object.h
    core/Result.h
    core/Value.h
    core/Visitor.h
    core/observer_ptr.h
    core/ref_ptr.h

    introspection/c_interface.h

    maths.h
    maths/mat4.h
    maths/vec2.h
    maths/vec3.h
    maths/vec4.h

    nodes.h
    nodes/Group.h
    nodes/LOD.h
    nodes/Node.h
    nodes/QuadGroup.h
    nodes/StateGroup.h

    utils.h
    utils/CommandLine.h
    utils/FileSystem.h
    utils/stream.h

    viewer.h
    viewer/GraphicsStage.h
    viewer/Viewer.h
    viewer/Window.h

    vk.h
    vk/BindVertexBuffers.h
    vk/BindindexBuffer.h
    vk/Buffer.h
    vk/BufferView.h
    vk/Command.h
    vk/CommandBuffer.h
    vk/CommandPool.h
    vk/CommandVisitor.h
    vk/ComputePipeline.h
    vk/Descriptor.h
    vk/DescriptorPool.h
    vk/DescriptorSet.h
    vk/DescriptorSetLayout.h
    vk/Device.h
    vk/DeviceMemory.h
    vk/Draw.h
    vk/Fence.h
    vk/Framebuffer.h
    vk/GraphicsPipeline.h
    vk/Image.h
    vk/ImageView.h
    vk/Instance.h
    vk/MemoryManager.h
    vk/PhysicalDevice.h
    vk/Pipeline.h
    vk/PipelineLayouyt.h
    vk/PushContants.h
    vk/Sampler.h
    vk/Semaphore.h
    vk/State.h
    vk/Surface.h
    vk/Swapchain.h
)

SET(SOURCES

    core/Auxiliary.cpp
    core/Object.cpp
    core/Result.cpp
    core/Visitor.cpp

    introspection/c_interface.cpp

    nodes/Group.cpp
    nodes/Node.cpp
    nodes/StateGroup.cpp

    utils/FileSystem.cpp

    viewer/GraphicsStage.cpp
    viewer/GLFW_Window.cpp
    viewer/Viewer.cpp
    viewer/Window.cpp

    vk/Buffer.cpp
    vk/BufferData.cpp
    vk/BufferView.cpp
    vk/BindIndexBuffer.cpp
    vk/BindVertexBuffers.cpp
    vk/CommandBuffer.cpp
    vk/CommandPool.cpp
    vk/CommandVisitor.cpp
    vk/ComputePipeline.cpp
    vk/DescriptorPool.cpp
    vk/DescriptorSet.cpp
    vk/DescriptorSetLayout.cpp
    vk/Device.cpp
    vk/DeviceMemory.cpp
    vk/Fence.cpp
    vk/Framebuffer.cpp
    vk/GraphicsPipeline.cpp
    vk/Image.cpp
    vk/ImageView.cpp
    vk/Instance.cpp
    vk/MemoryManager.cpp
    vk/PhysicalDevice.cpp
    vk/Pipeline.cpp
    vk/PipelineLayout.cpp
    vk/PushConstants.cpp
    vk/RenderPass.cpp
    vk/Sampler.cpp
    vk/Semaphore.cpp
    vk/ShaderModule.cpp
    vk/Surface.cpp
    vk/Swapchain.cpp
)

ADD_LIBRARY(vsg SHARED ${HEADER_FILES} ${SOURCES})

TARGET_LINK_LIBRARIES(vsg  ${Vulkan_LIBRARY} ${GLFW_LIBRARY})
