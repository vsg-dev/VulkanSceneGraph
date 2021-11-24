#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <deque>
#include <memory>

#include <vsg/core/ScratchMemory.h>
#include <vsg/nodes/Group.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/MemoryBufferPools.h>
#include <vsg/vk/ResourceRequirements.h>
#include <vsg/vk/ShaderCompiler.h>

#include <vsg/commands/Command.h>
#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/commands/CopyAndReleaseImage.h>

namespace vsg
{

    class VSG_DECLSPEC BuildAccelerationStructureCommand : public Inherit<Command, BuildAccelerationStructureCommand>
    {
    public:
        // the primitive Count is A) the amount of triangles to be built for type VK_GEOMETRY_TYPE_TRIANGLES_KHR (blas) B) the amount of AABBs for type VK_GEOMETRY_TYPE_AABBS_KHR
        // and C) the number of acceleration structures for type VK_GEOMETRY_TYPE_INSTANCES_KHR
        BuildAccelerationStructureCommand(Device* device, const VkAccelerationStructureBuildGeometryInfoKHR& info, const VkAccelerationStructureKHR& structure, const std::vector<uint32_t>& primitiveCounts, Allocator* allocator);

        void compile(Context&) override {}
        void record(CommandBuffer& commandBuffer) const override;
        void setScratchBuffer(ref_ptr<Buffer>& scratchBuffer);

        ref_ptr<Device> _device;
        VkAccelerationStructureBuildGeometryInfoKHR _accelerationStructureInfo;
        std::vector<VkAccelerationStructureGeometryKHR> _accelerationStructureGeometries;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR> _accelerationStructureBuildRangeInfos;
        VkAccelerationStructureKHR _accelerationStructure;

    protected:
        // scratch buffer set after compile traversal before record of build commands
        ref_ptr<Buffer> _scratchBuffer;
    };
    VSG_type_name(vsg::BuildAccelerationStructureCommand);

    class VSG_DECLSPEC Context : public Inherit<Object, Context>
    {
    public:
        Context();

        explicit Context(Device* in_device, const ResourceRequirements& resourceRequirements = {});

        Context(const Context& context);

        virtual ~Context();

        const uint32_t deviceID = 0;
        ref_ptr<Device> device;

        uint32_t viewID = 0;

        // get existing ShaderCompile or create a new one when GLSLang is supported
        ShaderCompiler* getOrCreateShaderCompiler();

        ref_ptr<CommandBuffer> getOrCreateCommandBuffer();

        // used by GraphicsPipeline.cpp
        ref_ptr<RenderPass> renderPass;

        // pipeline states that are usually not set in a scene, e.g.,
        // the viewport state, but might be set for some uses
        GraphicsPipelineStates defaultPipelineStates;

        // pipeline states that must be set to avoid Vulkan errors
        // e.g., MultisampleState.
        // XXX MultisampleState is complicated because the sample
        // number needs to agree with the renderpass attachment, but
        // other parts of the state, like alpha to coverage, belong to
        // the scene graph .
        GraphicsPipelineStates overridePipelineStates;

        // DescriptorPool
        ref_ptr<DescriptorPool> descriptorPool;

        // ShaderCompiler
        ref_ptr<ShaderCompiler> shaderCompiler;

        // transfer data settings
        ref_ptr<Queue> graphicsQueue;
        ref_ptr<CommandPool> commandPool;
        ref_ptr<CommandBuffer> commandBuffer;
        ref_ptr<Fence> fence;
        ref_ptr<Semaphore> semaphore;
        ref_ptr<ScratchMemory> scratchMemory;

        std::vector<ref_ptr<Command>> commands;

        ref_ptr<CopyAndReleaseImage> copyImageCmd;
        void copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest);
        void copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels);

        ref_ptr<CopyAndReleaseBuffer> copyBufferCmd;
        void copy(ref_ptr<BufferInfo> src, ref_ptr<BufferInfo> dest);

        /// return true if there are commands that have been submitted
        bool record();

        void waitForCompletion();

        ref_ptr<MemoryBufferPools> deviceMemoryBufferPools;
        ref_ptr<MemoryBufferPools> stagingMemoryBufferPools;

        // RTX ray tracing
        VkDeviceSize scratchBufferSize;
        std::vector<ref_ptr<BuildAccelerationStructureCommand>> buildAccelerationStructureCommands;
    };
    VSG_type_name(vsg::Context);

} // namespace vsg
