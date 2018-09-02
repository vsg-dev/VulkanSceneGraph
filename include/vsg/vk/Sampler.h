#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{
    class VSG_EXPORT Sampler : public Object
    {
    public:
        Sampler(VkSampler Sampler, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Sampler, VkResult, VK_SUCCESS>;
        static Result create(Device* device, const VkSamplerCreateInfo& createSamplerInfo, AllocationCallbacks* allocator=nullptr);

        static Result create(Device* device, AllocationCallbacks* allocator=nullptr);

        VkSampler sampler() const { return _sampler; }

        operator VkSampler () const { return _sampler; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~Sampler();

        VkSampler                       _sampler;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
