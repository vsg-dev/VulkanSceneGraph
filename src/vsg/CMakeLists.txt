
INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
)

SET(SOURCES

    core/Auxiliary.cpp
    core/Object.cpp
    core/Result.cpp
    core/Visitor.cpp

    introspection/c_interface.cpp

    nodes/Group.cpp
    nodes/Node.cpp
    nodes/QuadGroup.cpp
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
if   (VSG_SHARED_LIBRARY)
    add_definitions(-DVSG_LIBRARY)
    add_library(vsg SHARED ${SOURCES})
else ()
    add_definitions(-DVSG_LIBRARY_STATIC)
    add_library(vsg STATIC ${SOURCES})
endif()

target_link_libraries(vsg ${Vulkan_LIBRARY} ${GLFW_LIBRARY})

install(TARGETS vsg
        LIBRARY
            DESTINATION lib
            COMPONENT libvsg
)


install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/vsg DESTINATION include)

