include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
)

set(SOURCES

    core/Allocator.cpp
    core/Auxiliary.cpp
    core/Object.cpp
    core/Result.cpp
    core/Visitor.cpp
    core/ConstVisitor.cpp
    core/Version.cpp

    introspection/c_interface.cpp

    nodes/Group.cpp
    nodes/Node.cpp
    nodes/QuadGroup.cpp
    nodes/StateGroup.cpp

    utils/FileSystem.cpp

    traversals/DispatchTraversal.cpp
    traversals/CullTraversal.cpp

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

if (BUILD_SHARED_LIBS)
    add_definitions(-DVSG_LIBRARY)
    add_library(vsg SHARED ${SOURCES})
else()
    add_definitions(-DVSG_LIBRARY_STATIC)
    add_library(vsg STATIC ${SOURCES})
endif()

target_link_libraries(vsg ${Vulkan_LIBRARY} ${GLFW_LIBRARY})

set_target_properties(vsg PROPERTIES VERSION ${VSG_MAJOR_VERSION}.${VSG_MINOR_VERSION}.${VSG_PATCH_VERSION} SOVERSION ${VSG_SOVERSION})

install(TARGETS vsg
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        COMPONENT libvsg
)


install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/vsg DESTINATION include)

