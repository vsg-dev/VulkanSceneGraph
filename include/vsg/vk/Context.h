#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <deque>
#include <memory>

#include <vsg/core/Object.h>

#include <vsg/nodes/Group.h>

#include <vsg/vk/BufferData.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/Semaphore.h>

#include <vsg/vk/BufferData.h>
#include <vsg/vk/ImageData.h>

namespace vsg
{
    struct BufferPreferences
    {
        VkDeviceSize minimumBufferSize = 16 * 1024 * 1024;
        VkDeviceSize minimumBufferDeviceMemorySize = 16 * 1024 * 1024;
        VkDeviceSize minimumImageDeviceMemorySize = 16 * 1024 * 1024;
    };

    class MemoryBufferPools : public Inherit<Object, MemoryBufferPools>
    {
    public:
        MemoryBufferPools(const std::string& name, Device* in_device, BufferPreferences preferences);

        std::string name;
        ref_ptr<Device> device;
        BufferPreferences bufferPreferences;

        // transfer data settings
        // used by BufferData.cpp, ImageData.cpp
        using MemoryPools = std::vector<ref_ptr<DeviceMemory>>;
        MemoryPools memoryPools;

        using BufferPools = std::vector<ref_ptr<Buffer>>;
        BufferPools bufferPools;

        VkDeviceSize computeMemoryTotalAvailble() const;
        VkDeviceSize computeMemoryTotalReserved() const;
        VkDeviceSize computeBufferTotalAvailble() const;
        VkDeviceSize computeBufferTotalReserved() const;

        BufferData reserveBufferData(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties);

        using DeviceMemoryOffset = std::pair<ref_ptr<DeviceMemory>, VkDeviceSize>;
        DeviceMemoryOffset reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryProperties, void* pNextAllocInfo = nullptr);

        using CopyPair = std::pair<BufferData, BufferData>;
        using CopyQueue = std::deque<CopyPair>;

        CopyQueue bufferDataToCopy;
    };

    class CopyAndReleaseBufferDataCommand : public Command
    {
    public:
        CopyAndReleaseBufferDataCommand(BufferData src, BufferData dest) :
            source(src),
            destination(dest) {}

        BufferData source;
        BufferData destination;

        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~CopyAndReleaseBufferDataCommand();
    };

    class CopyAndReleaseImageDataCommand : public Command
    {
    public:
        CopyAndReleaseImageDataCommand(BufferData src, ImageData dest, uint32_t numMipMapLevels) :
            source(src),
            destination(dest),
            mipLevels(numMipMapLevels) {}

        void dispatch(CommandBuffer& commandBuffer) const override;

        BufferData source;
        ImageData destination;
        uint32_t mipLevels = 1;

    protected:
        virtual ~CopyAndReleaseImageDataCommand();
    };

    class BuildAccelerationStructureCommand : public Inherit<Command, BuildAccelerationStructureCommand>
    {
    public:
        BuildAccelerationStructureCommand(Device* device, VkAccelerationStructureInfoNV* info, const VkAccelerationStructureNV& structure, Buffer* instanceBuffer, Allocator* allocator = nullptr);

        void compile(Context&) override {}
        void dispatch(CommandBuffer& commandBuffer) const override;

        ref_ptr<Device> _device;
        VkAccelerationStructureInfoNV* _accelerationStructureInfo;
        VkAccelerationStructureNV _accelerationStructure;
        ref_ptr<Buffer> _instanceBuffer;

        // scratch buffer set after compile traversal before dispatch of build commands
        ref_ptr<Buffer> _scratchBuffer;
    };

    class Context
    {
    public:
        Context(Device* in_device, BufferPreferences bufferPreferences = {});

        Context(const Context& context);

        virtual ~Context();

        // used by BufferData.cpp, ComputePipeline.cpp, Descriptor.cpp, Descriptor.cpp, DescriptorSet.cpp, DescriptorSetLayout.cpp, GraphicsPipeline.cpp, ImageData.cpp, PipelineLayout.cpp, ShaderModule.cpp
        ref_ptr<Device> device;

        // used by GraphicsPipeline.cpp
        ref_ptr<RenderPass> renderPass;
        ref_ptr<ViewportState> viewport;

        // DescriptorSet.cpp
        ref_ptr<DescriptorPool> descriptorPool;

        // transfer data settings
        // used by BufferData.cpp, ImageData.cpp
        ref_ptr<Queue> graphicsQueue;
        ref_ptr<CommandPool> commandPool;
        ref_ptr<CommandBuffer> commandBuffer;
        ref_ptr<Fence> fence;
        ref_ptr<Semaphore> semaphore;

        std::vector<ref_ptr<CopyAndReleaseBufferDataCommand>> copyBufferDataCommands;
        std::vector<ref_ptr<CopyAndReleaseImageDataCommand>> copyImageDataCommands;
        std::vector<ref_ptr<Command>> commands;

        void dispatch();
        void waitForCompletion();

        ref_ptr<CommandBuffer> getOrCreateCommandBuffer();

        ref_ptr<MemoryBufferPools> deviceMemoryBufferPools;
        ref_ptr<MemoryBufferPools> stagingMemoryBufferPools;

        // raytracing
        VkDeviceSize scratchBufferSize;
        std::vector<ref_ptr<BuildAccelerationStructureCommand>> buildAccelerationStructureCommands;
    };

} // namespace vsg
