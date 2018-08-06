#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/PipelineLayout.h>

namespace vsg
{

    using DescriptorPoolSizes = std::vector<VkDescriptorPoolSize>;
    using DescriptorBufferInfos = std::vector<VkDescriptorBufferInfo>;
    using DescriptorSets = std::vector<VkDescriptorSet>;

    class DescriptorPool : public vsg::Object
    {
    public:
        DescriptorPool(Device* device, VkDescriptorPool descriptorPool, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<DescriptorPool, VkResult, VK_SUCCESS>;

        static Result create(Device* device, uint32_t maxSets, const DescriptorPoolSizes& descriptorPoolSizes, AllocationCallbacks* allocator=nullptr);

        operator const VkDescriptorPool& () const { return _descriptorPool; }

    protected:
        virtual ~DescriptorPool();

        ref_ptr<Device>                 _device;
        VkDescriptorPool                _descriptorPool;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

    class CmdBindDescriptorSets : public Command
    {
    public:

        CmdBindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, const DescriptorSets& descriptorSets) :
            _bindPoint(bindPoint),
            _pipelineLayout(pipelineLayout),
            _descriptorSets(descriptorSets) {}

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindDescriptorSets(commandBuffer, _bindPoint, *_pipelineLayout, 0, _descriptorSets.size(), _descriptorSets.data(), 0, nullptr);
        }

    protected:
        virtual ~CmdBindDescriptorSets() {}

        VkPipelineBindPoint         _bindPoint;
        ref_ptr<PipelineLayout>     _pipelineLayout;
        DescriptorSets              _descriptorSets;
    };

}
