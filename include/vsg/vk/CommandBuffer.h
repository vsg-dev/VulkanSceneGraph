#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ScratchMemory.h>
#include <vsg/state/ComputePipeline.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/vk/CommandPool.h>

namespace vsg
{

    class VSG_DECLSPEC CommandBuffer : public Inherit<Object, CommandBuffer>
    {
    public:
        CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        const VkCommandBuffer* data() const { return &_commandBuffer; }

        operator VkCommandBuffer() const { return _commandBuffer; }

        std::atomic_uint& numDependentSubmissions() { return _numDependentSubmissions; }

        const uint32_t deviceID;

        VkCommandBufferLevel level() const {return _level; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        CommandPool* getCommandPool() { return _commandPool; }
        const CommandPool* getCommandPool() const { return _commandPool; }

        void setCurrentPipelineLayout(VkPipelineLayout pipelineLayout) { _currentPipelineLayout = pipelineLayout; }
        VkPipelineLayout getCurrentPipelineLayout() const { return _currentPipelineLayout; }

        ref_ptr<ScratchMemory> scratchMemory;

    protected:
        virtual ~CommandBuffer();

        VkCommandBuffer _commandBuffer;
        VkCommandBufferLevel _level;

        std::atomic_uint _numDependentSubmissions{0};
        ref_ptr<Device> _device;
        ref_ptr<CommandPool> _commandPool;
        VkPipelineLayout _currentPipelineLayout;
    };
    VSG_type_name(vsg::CommandBuffer);

    using CommandBuffers = std::vector<ref_ptr<CommandBuffer>>;

} // namespace vsg
