# include/vsg/vk headers
The **include/vsg/vk** header directory contains the Vulkan C API integration classes. The integration class add support for reference counting of Vulkan objects and automatic lifetime management to ensure that Vukan objects can not be deleted while they are still be used, and finally automatic clean up was once the references are removed.

## High Level Vulkan integration classes

High level Vulkan integration concerns Vulkan objects that are created at the Application, Window and Viewer level and don't change as scene graph/command graph level Vulkan objects are created and destroyed.

* [include/vsg/vk/Instance.h](Instance.h) - wrapper for [vkInstance](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkInstance.html)
* [include/vsg/vk/PhysicalDevice.h](PhysicalDevice.h) - wrapper for [vkPhysicalDevice](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPhysicalDevice.html)
* [include/vsg/vk/Surface.h](Surface.h) - wrapper for vkSurface
* [include/vsg/vk/Device.h](Device.h) -
* [include/vsg/vk/RenderPass.h](RenderPass.h) -
* [include/vsg/vk/Semaphore.h](Semaphore.h) -
* [include/vsg/vk/Swapchain.h](Swapchain.h) -
* [include/vsg/vk/CommandBuffer.h](CommandBuffer.h) -
* [include/vsg/vk/CommandPool.h](CommandPool.h) -
* [include/vsg/vk/CommandVisitor.h](CommandVisitor.h) -
* [include/vsg/vk/Fence.h](Fence.h) -
* [include/vsg/vk/Framebuffer.h](Framebuffer.h) -
* [include/vsg/vk/State.h](State.h) -

## Low level Vulkan integration classes

Low level Vulkan integration concern Vulkan objects that relate to data and commands defined in Scene Graphs and Command Graphs.


* [include/vsg/vk/Command.h](Command.h) -
* [include/vsg/vk/BindIndexBuffer.h](BindIndexBuffer.h) -
* [include/vsg/vk/BindVertexBuffers.h](BindVertexBuffers.h) -
* [include/vsg/vk/BufferData.h](BufferData.h) -
* [include/vsg/vk/Buffer.h](Buffer.h) -
* [include/vsg/vk/Buffer.h](Buffer.h) -
* [include/vsg/vk/Descriptor.h](Descriptor.h) -
* [include/vsg/vk/DescriptorPool.h](DescriptorPool.h) -
* [include/vsg/vk/DescriptorSet.h](DescriptorSet.h) -
* [include/vsg/vk/DescriptorSetLayout.h](DescriptorSetLayout.h) -
* [include/vsg/vk/DeviceMemory.h](DeviceMemory.h) -
* [include/vsg/vk/Draw.h](Draw.h) -
* [include/vsg/vk/Image.h](Image.h) -
* [include/vsg/vk/ImageView.h](ImageView.h) -
* [include/vsg/vk/Pipeline.h](Pipeline.h) -
* [include/vsg/vk/PipelineLayout.h](PipelineLayout.h) -
* [include/vsg/vk/GraphicsPipeline.h](GraphicsPipeline.h) -
* [include/vsg/vk/ComputePipeline.h](ComputePipeline.h) -
* [include/vsg/vk/PushConstants.h](PushConstants.h) -
* [include/vsg/vk/Sampler.h](Sampler.h) -
* [include/vsg/vk/ShaderModule.h](ShaderModule.h) -

## Memory management classes

* [include/vsg/vk/AllocationCallbacks.h](AllocationCallbacks.h) -
* [include/vsg/vk/MemoryManager.h](MemoryManager.h) -
