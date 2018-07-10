#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/vk/AllocationCallbacks.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace vsg
{
    using Names = std::vector<const char*>;

    extern Names validateInstancelayerNames(const Names& names);

    inline VkQueue createDeviceQueue(VkDevice device, int graphicsFamily)
    {
        VkQueue queue;
        vkGetDeviceQueue(device, graphicsFamily, 0, &queue);
        return queue;
    }

    class Instance : public vsg::Object
    {
    public:
        Instance(VkInstance instance, AllocationCallbacks* allocator=nullptr) : _instance(instance), _allocator(allocator) {}

        Instance(Names& instanceExtensions, Names& layers, AllocationCallbacks* allocator=nullptr);

        operator VkInstance() const { return _instance; }
        VkInstance getInstance() const { return _instance; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

    protected:

        virtual ~Instance();

        VkInstance                      _instance;
        ref_ptr<AllocationCallbacks>    _allocator;
    };
}
