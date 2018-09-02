#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/vk/AllocationCallbacks.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace vsg
{
    using Names = std::vector<const char*>;

    extern VSG_EXPORT Names validateInstancelayerNames(const Names& names);

    class VSG_EXPORT Instance : public vsg::Object
    {
    public:
        Instance(VkInstance instance, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Instance, VkResult, VK_SUCCESS>;
        static Result create(Names& instanceExtensions, Names& layers, AllocationCallbacks* allocator=nullptr);

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
