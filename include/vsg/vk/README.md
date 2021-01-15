# include/vsg/vk headers
The **include/vsg/vk** header directory contains the Vulkan C application level integration classes.

## Naming convention
The Vulkan integration wrappers follow the convention **VkName -> vsg::Name** with the wrapper class found in the header **include/vsg/vk/Name.h**. For example **VkInstance** is wrapped by the class vsg::Instance which is located in header [include/vsg/vk/Instance.h](Instance.h).

## Vulkan object creation and validity
The Vulkan integration wrappers create their associated Vulkan objects in their respective constructors and destroy these Vulkan object their destructors.  The combination of the VulkanClass::create and destructor behaviour ensures that the associated Vulkan object is valid for the whole lifetime of wrapper object.

## Memory management
The Vulkan integration classes add support automatic lifetime management to ensure that Vukan objects can not be deleted while they are still be used, and finally automatic clean up was once the references are removed.

The lifetime management is provided by leveraging the vsg's intrusive reference counting support provided by vsg::ref_ptr<> and vsg::Object base class. To ensure that higher level Vulkan objects (vkDevice/vsg::Device etc.) are not deleted before lower level Vulkan objects (vsg::CommandPool, vsg::BufferData etc.) are still using/reference them the lower level Vulkan wrapper classes hold a vsg::ref_ptr<> reference to the high level Vulkan wrapper classes.

The scheme of lower level Vulkan wrappers holding reference to high level Vulkan Wrappers is outwardly the inverse of how one would normally think of ownership hierarchy, which would be along the lines of an Instance owning a list logical Devices, but the power behind this scheme is it enables decoupled, thread safe and robust lifetime management whilst remaining simple to implement and easy to use. The following pseudo code illustrates:

```c++
{
    vsg::ref_ptr<vsg::Instance> instance = vsg::Instance::create(...);
    vsg::ref_ptr<vsg::Device> device = vsg::Device::create(instance,...); // device holds a ref_ptr<> to instance

   // even if we try to discard the instance explicitly,
   // or it goes out of scope things remain safe
   instance = nullptr; // Instance object isn't deleted, as Device still needs it

   ...
   // application code using Device
   ...

} // device goes out of scope, both Device and Instance automatically
  // cleaned up in the correct order : VkDevice then VkInstance.
```

To see an example of memory management working in a full blown code see the [vsgdraw](https://github.com/vsg-dev/vsgExamples/tree/master/examples/commands/vsgdraw) found in [vsgExample](https://github.com/vsg-dev/vsgExamples) repository. The key is there won't asee any explicit management of lifetime, it's all done for you when all the ref_ptr<> go out of scope at the end of main. To see that Vulkan is being cleaned up correctly run this example with the --api command line thus: ```vsgdraw --api``` to see all Vulkan API calls output to the console.

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
* [include/vsg/vk/Fence.h](Fence.h) -
* [include/vsg/vk/Framebuffer.h](Framebuffer.h) -
* [include/vsg/vk/State.h](State.h) -
* [include/vsg/vk/AllocationCallbacks.h](AllocationCallbacks.h) -
