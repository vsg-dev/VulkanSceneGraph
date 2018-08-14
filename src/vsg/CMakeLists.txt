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
    core/Result.h

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
    vk/Command.h
    vk/Surface.h
    vk/ImageView.h
    vk/BufferView.h
    vk/Swapchain.h
    vk/CmdDraw.h
    vk/Pipeline.h
    vk/ComputePipeline.h
    vk/GraphicsPipeline.h
    vk/PipelineLayouyt.h
    vk/Framebuffer.h
    vk/CommandBuffer.h
    vk/CommandPool.h
    vk/CommandVisitor.h
    vk/Semaphore.h
    vk/DeviceMemory.h
    vk/MemoryManager.h
    vk/DescriptorPool.h
    vk/Descriptor.h
    vk/DescriptorSet.h
    vk/DescriptorSetLayout.h
    vk/Buffer.h
    vk/BindVertexBuffers.h
    vk/BindindexBuffer.h
    vk/Image.h
    vk/Sampler.h
    vk/Fence.h

    viewer/Window.h
    viewer/Viewer.h

    utils/CommandLine.h
)

SET(SOURCES

    core/Auxiliary.cpp
    core/Object.cpp
    core/Visitor.cpp
    core/Result.cpp

    nodes/Group.cpp
    nodes/Node.cpp

    vk/PhysicalDevice.cpp
    vk/Device.cpp
    vk/Surface.cpp
    vk/ImageView.cpp
    vk/BufferView.cpp
    vk/BufferData.cpp
    vk/Instance.cpp
    vk/Swapchain.cpp
    vk/ShaderModule.cpp
    vk/RenderPass.cpp
    vk/Pipeline.cpp
    vk/ComputePipeline.cpp
    vk/GraphicsPipeline.cpp
    vk/PipelineLayout.cpp
    vk/Framebuffer.cpp
    vk/CommandBuffer.cpp
    vk/CommandPool.cpp
    vk/CommandVisitor.cpp
    vk/Semaphore.cpp
    vk/DeviceMemory.cpp
    vk/MemoryManager.cpp
    vk/DescriptorPool.cpp
    vk/DescriptorSet.cpp
    vk/DescriptorSetLayout.cpp
    vk/Buffer.cpp
    vk/Image.cpp
    vk/Sampler.cpp
    vk/Fence.cpp

    viewer/Window.cpp
    viewer/Viewer.cpp
)

ADD_LIBRARY(vsg SHARED ${HEADER_FILES} ${SOURCES})

TARGET_LINK_LIBRARIES(vsg  ${Vulkan_LIBRARY})
