include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

SET(HEADER_PATH ${CMAKE_SOURCE_DIR}/include/vsg)

set(HEADERS
    ${HEADER_PATH}/all.h
    ${CMAKE_BINARY_DIR}/include/vsg/core/Version.h
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

add_definitions(${VSG_DEFINITIONS})

add_library(vsg ${HEADERS} ${SOURCES})

target_link_libraries(vsg Vulkan::Vulkan GLFW::GLFW)

set_target_properties(vsg
        PROPERTIES VERSION ${VSG_MAJOR_VERSION}.${VSG_MINOR_VERSION}.${VSG_PATCH_VERSION}
        SOVERSION ${VSG_SOVERSION}
)

install(TARGETS vsg
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        COMPONENT libvsg
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/vsg DESTINATION include)

if (NOT(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR}))
    install(DIRECTORY ${CMAKE_BINARY_DIR}/include/vsg DESTINATION include)
endif()
