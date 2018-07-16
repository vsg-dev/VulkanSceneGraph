#pragma once

#include <vsg/vk/CommandPool.h>

namespace vsg
{
    class CommandBuffers : public Object
    {
    public:
        using Buffers = std::vector<VkCommandBuffer>;

        CommandBuffers(Device* device, CommandPool* commandPool, const Buffers& buffers);

        using Result = vsg::Result<CommandBuffers, VkResult, VK_SUCCESS>;
        static Result create(Device* device,  CommandPool* commandPool, size_t size);

        std::size_t size() const { return _buffers.size(); }
        const VkCommandBuffer& operator [] (size_t i) const { return _buffers[i]; }
        const VkCommandBuffer& at(size_t i) const { return _buffers[i]; }

    protected:
        virtual ~CommandBuffers();

        ref_ptr<Device>         _device;
        ref_ptr<CommandPool>    _commandPool;
        Buffers                 _buffers;
    };

}
