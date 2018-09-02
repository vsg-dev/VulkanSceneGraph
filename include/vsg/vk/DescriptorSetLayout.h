#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{
    using DescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>;

    class VSG_EXPORT DescriptorSetLayout : public vsg::Object
    {
    public:
        DescriptorSetLayout(Device* device, VkDescriptorSetLayout DescriptorSetLayout, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<DescriptorSetLayout, VkResult, VK_SUCCESS>;

        static Result create(Device* device, const DescriptorSetLayoutBindings& descriptorSetLayoutBindings, AllocationCallbacks* allocator=nullptr);

        operator const VkDescriptorSetLayout& () const { return _descriptorSetLayout; }

    protected:
        virtual ~DescriptorSetLayout();

        ref_ptr<Device>                 _device;
        VkDescriptorSetLayout           _descriptorSetLayout;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
