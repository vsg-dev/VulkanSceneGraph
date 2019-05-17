#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <memory>
#include <vsg/core/Object.h>

#include <vsg/nodes/Group.h>

#include <vsg/vk/CommandPool.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/GraphicsPipeline.h>

namespace vsg
{
    class Context
    {
    public:
        ref_ptr<Device> device;
        ref_ptr<CommandPool> commandPool;
        ref_ptr<RenderPass> renderPass;
        ref_ptr<ViewportState> viewport;
        VkQueue graphicsQueue = 0;

        ref_ptr<DescriptorPool> descriptorPool;

        ref_ptr<mat4Value> projMatrix;
        ref_ptr<mat4Value> viewMatrix;
#if 1
        VkDeviceSize minimumBufferSize = 16 * 1024 * 1024;
        VkDeviceSize minimumBufferDeviceMemorySize = 16 * 1024 * 1024;
        VkDeviceSize minimumImageDeviceMemorySize = 16 * 1024 * 1024;
#else
        VkDeviceSize minimumBufferSize = 1;             //1024 * 1024;
        VkDeviceSize minimumBufferDeviceMemorySize = 1; //1024 * 1024;
        VkDeviceSize minimumImageDeviceMemorySize = 1;  //1024 * 1024;
#endif

        using MemoryPools = std::vector<ref_ptr<DeviceMemory>>;
        MemoryPools memoryPools;

        using BufferPools = std::vector<ref_ptr<Buffer>>;
        BufferPools bufferPools;
    };

    class VSG_DECLSPEC CompileTraversal : public Visitor
    {
    public:
        explicit CompileTraversal();
        ~CompileTraversal();

        void apply(Object& object);
        void apply(Command& command);
        void apply(Commands& commands);
        void apply(StateGroup& stateGroup);
        void apply(Geometry& geometry);

        Context context;
    };
} // namespace vsg
