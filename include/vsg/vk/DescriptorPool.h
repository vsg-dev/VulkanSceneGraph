#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{

    using DescriptorPoolSizes = std::vector<VkDescriptorPoolSize>;

    class DescriptorPool : public vsg::Object
    {
    public:
        DescriptorPool(VkDescriptorPool descriptorPool, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<DescriptorPool, VkResult, VK_SUCCESS>;

        static Result create(Device* device, uint32_t maxSets, const DescriptorPoolSizes& descriptorPoolSizes, AllocationCallbacks* allocator=nullptr);

        operator const VkDescriptorPool& () const { return _descriptorPool; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~DescriptorPool();

        VkDescriptorPool                _descriptorPool;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };
}
