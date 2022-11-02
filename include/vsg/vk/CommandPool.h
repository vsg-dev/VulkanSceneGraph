#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>

namespace vsg
{
    // forward declare
    class CommandBuffer;

    /// CommandPool encapsulates vkCommandPool.
    /// CommandPool are used in Vulkan as a source of Commands that are recorded to a CommandBuffer during the RecordTraversal, or dedicated command submissions.
    class VSG_DECLSPEC CommandPool : public Inherit<Object, CommandPool>
    {
    public:
        CommandPool(Device* device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

        operator VkCommandPool() const { return _commandPool; }
        VkCommandPool vk() const { return _commandPool; }

        void reset(VkCommandPoolResetFlags flags = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) const { vkResetCommandPool(*_device, _commandPool, flags); }

        /// allocate CommandBuffer from CommandPool
        ref_ptr<CommandBuffer> allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~CommandPool();

        friend CommandBuffer;

        /// free CommandBuffer
        void free(CommandBuffer* commandBuffer);

        std::mutex _mutex;
        VkCommandPool _commandPool;
        ref_ptr<Device> _device;
    };
    VSG_type_name(vsg::CommandPool);

} // namespace vsg
