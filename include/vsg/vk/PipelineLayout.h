#pragma once

#include <vsg/vk/DescriptorSetLayout.h>

namespace vsg
{

    using DescriptorSetLayouts = std::vector<ref_ptr<DescriptorSetLayout>>;
    using PushConstantRanges = std::vector<VkPushConstantRange>;

    class VSG_EXPORT PipelineLayout : public vsg::Object
    {
    public:
        PipelineLayout(VkPipelineLayout pipelineLayout, const DescriptorSetLayouts& descrtorSetLayouts, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<PipelineLayout, VkResult, VK_SUCCESS>;

        static Result create(Device* device, const DescriptorSetLayouts& descriptorSetLayouts, const PushConstantRanges& pushConstantRanges, VkPipelineLayoutCreateFlags flags=0, AllocationCallbacks* allocator=nullptr);

        operator VkPipelineLayout () const { return _pipelineLayout; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~PipelineLayout();

        VkPipelineLayout                _pipelineLayout;
        DescriptorSetLayouts            _descriptorSetLayouts;

        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
