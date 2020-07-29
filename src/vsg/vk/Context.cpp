/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/commands/Commands.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/state/StateGroup.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Extensions.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <iostream>

#define REPORT_STATS 0

#if REPORT_STATS
#    include <chrono>
#endif

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::CopyAndReleaseBufferDataCommand
//
CopyAndReleaseBufferDataCommand::~CopyAndReleaseBufferDataCommand()
{
    source.release();
}

void CopyAndReleaseBufferDataCommand::record(CommandBuffer& commandBuffer) const
{
    //std::cout<<"CopyAndReleaseBufferDataCommand::record(CommandBuffer& commandBuffer) source.offset = "<<source.offset<<", "<<destination.offset<<std::endl;
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = source.offset;
    copyRegion.dstOffset = destination.offset;
    copyRegion.size = source.range;
    vkCmdCopyBuffer(commandBuffer, *source.buffer, *destination.buffer, 1, &copyRegion);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::CopyAndReleaseImageDataCommand
//
CopyAndReleaseImageDataCommand::~CopyAndReleaseImageDataCommand()
{
    source.release();
}

void CopyAndReleaseImageDataCommand::record(CommandBuffer& commandBuffer) const
{
    ref_ptr<Buffer> imageStagingBuffer(source.buffer);
    ref_ptr<Data> data(source.data);
    ref_ptr<Image> textureImage(destination.imageView->getImage());
    ref_ptr<Sampler> sampler(destination.sampler);
    VkImageLayout targetImageLayout = destination.imageLayout;

    Data::Layout layout = data->getLayout();
    auto mipmapOffsets = data->computeMipmapOffsets();

    bool useDataMipmaps = (mipLevels > 1) && (mipmapOffsets.size() > 1);
    bool generatMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);

    uint32_t width = data->width() * layout.blockWidth;
    uint32_t height = data->height() * layout.blockHeight;
    uint32_t depth = data->depth() * layout.blockDepth;

    // transfer the data.
    if (useDataMipmaps)
    {
        VkImageMemoryBarrier preCopyBarrier = {};
        preCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        preCopyBarrier.srcAccessMask = 0;
        preCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        preCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        preCopyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        preCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.image = *textureImage;
        preCopyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        preCopyBarrier.subresourceRange.baseArrayLayer = 0;
        preCopyBarrier.subresourceRange.layerCount = 1;
        preCopyBarrier.subresourceRange.levelCount = mipLevels;
        preCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &preCopyBarrier);

        uint32_t mipWidth = width;
        uint32_t mipHeight = height;
        uint32_t mipDepth = depth;
        auto valueSize = data->valueSize();
        for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
        {
            // std::cout<<"   level = "<<mipLevel<<", mipWidth = "<<mipWidth<<", mipHeight = "<<mipHeight<<std::endl;
            VkBufferImageCopy region = {};
            region.bufferOffset = source.offset + mipmapOffsets[mipLevel] * valueSize;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {mipWidth, mipHeight, mipDepth};

            vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
            if (mipDepth > 1) mipDepth /= 2;
        }

        VkImageMemoryBarrier postCopyBarrier = {};
        postCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        postCopyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        postCopyBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        postCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        postCopyBarrier.newLayout = targetImageLayout;
        postCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        postCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        postCopyBarrier.image = *textureImage;
        postCopyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        postCopyBarrier.subresourceRange.baseArrayLayer = 0;
        postCopyBarrier.subresourceRange.layerCount = 1;
        postCopyBarrier.subresourceRange.levelCount = mipLevels;
        postCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &postCopyBarrier);
    }
    else if (generatMipmaps)
    {
        // generate mipmaps using Vulkan
        VkImageMemoryBarrier preCopyBarrier = {};
        preCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        preCopyBarrier.srcAccessMask = 0;
        preCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        preCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        preCopyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        preCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.image = *textureImage;
        preCopyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        preCopyBarrier.subresourceRange.baseArrayLayer = 0;
        preCopyBarrier.subresourceRange.layerCount = 1;
        preCopyBarrier.subresourceRange.levelCount = mipLevels;
        preCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &preCopyBarrier);

        VkBufferImageCopy region = {};
        region.bufferOffset = source.offset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, depth};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = *textureImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = width;
        int32_t mipHeight = height;
        int32_t mipDepth = depth;

        for (uint32_t i = 1; i < mipLevels; ++i)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit = {};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                           *textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = targetImageLayout;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
            if (mipDepth > 1) mipDepth /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = targetImageLayout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
    }
    else
    {

        auto preCopyImageMemoryBarrier = ImageMemoryBarrier::create(
            0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            textureImage,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        auto preCopyPipelineBarrier = PipelineBarrier::create(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, preCopyImageMemoryBarrier);

        preCopyPipelineBarrier->record(commandBuffer);

        VkBufferImageCopy region = {};
        region.bufferOffset = source.offset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, depth};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        auto postCopyImageBarrier = ImageMemoryBarrier::create(
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetImageLayout,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            textureImage,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        auto postPipelineBarrier = PipelineBarrier::create(
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, postCopyImageBarrier);

        postPipelineBarrier->record(commandBuffer);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// BuildAccelerationStructureCommand
//

BuildAccelerationStructureCommand::BuildAccelerationStructureCommand(Device* device, VkAccelerationStructureInfoNV* info, const VkAccelerationStructureNV& structure, Buffer* instanceBuffer, Allocator* allocator) :
    Inherit(allocator),
    _device(device),
    _accelerationStructureInfo(info),
    _accelerationStructure(structure),
    _instanceBuffer(instanceBuffer)
{
}

void BuildAccelerationStructureCommand::record(CommandBuffer& commandBuffer) const
{
    Extensions* extensions = Extensions::Get(_device, true);

    extensions->vkCmdBuildAccelerationStructureNV(commandBuffer,
                                                  _accelerationStructureInfo,
                                                  _instanceBuffer.valid() ? *_instanceBuffer : (VkBuffer)VK_NULL_HANDLE,
                                                  0,
                                                  VK_FALSE,
                                                  _accelerationStructure,
                                                  VK_NULL_HANDLE,
                                                  *_scratchBuffer,
                                                  0);

    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Context
//
Context::Context(Device* in_device, BufferPreferences bufferPreferences) :
    deviceID(in_device->deviceID),
    device(in_device),
    deviceMemoryBufferPools(MemoryBufferPools::create("Device_MemoryBufferPool", device, bufferPreferences)),
    stagingMemoryBufferPools(MemoryBufferPools::create("Staging_MemoryBufferPool", device, bufferPreferences)),
    scratchBufferSize(0)
{
    //semaphore = vsg::Semaphore::create(device);
    scratchMemory = ScratchMemory::create(4096);
}

Context::Context(const Context& context) :
    deviceID(context.deviceID),
    device(context.device),
    renderPass(context.renderPass),
    defaultPipelineStates(context.defaultPipelineStates),
    overridePipelineStates(context.overridePipelineStates),
    descriptorPool(context.descriptorPool),
    graphicsQueue(context.graphicsQueue),
    commandPool(context.commandPool),
    deviceMemoryBufferPools(context.deviceMemoryBufferPools),
    stagingMemoryBufferPools(context.stagingMemoryBufferPools),
    scratchBufferSize(context.scratchBufferSize)
{
    scratchMemory = ScratchMemory::create(4096);
}

Context::~Context()
{
    waitForCompletion();
}

ref_ptr<CommandBuffer> Context::getOrCreateCommandBuffer()
{
    if (!commandBuffer)
    {
        commandBuffer = vsg::CommandBuffer::create(device, commandPool);
    }

    return commandBuffer;
}

void Context::record()
{
    if (commands.empty() && buildAccelerationStructureCommands.empty()) return;

    //auto before_compile = std::chrono::steady_clock::now();

    if (!fence)
    {
        fence = vsg::Fence::create(device);
    }
    else
    {
        fence->reset();
    }

    getOrCreateCommandBuffer();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(*commandBuffer, &beginInfo);

    // issue commands of interest
    {
        for (auto& command : commands) command->record(*commandBuffer);
    }

    // create scratch buffer and issue build acceleration sctructure commands
    ref_ptr<Buffer> scratchBuffer;
    ref_ptr<DeviceMemory> scratchBufferMemory;
    if (scratchBufferSize > 0)
    {
        scratchBuffer = Buffer::create(device, scratchBufferSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_SHARING_MODE_EXCLUSIVE);

        scratchBufferMemory = vsg::DeviceMemory::create(device, scratchBuffer->getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        scratchBuffer->bind(scratchBufferMemory, 0);

        for (auto& command : buildAccelerationStructureCommands)
        {
            command->_scratchBuffer = scratchBuffer;
            command->record(*commandBuffer);
        }
    }

    vkEndCommandBuffer(*commandBuffer);

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer->data();
    if (semaphore)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = semaphore->data();
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
    }
    else
    {
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
    }

    graphicsQueue->submit(submitInfo, fence);
    //std::cout << "Context::recordCommands()  time " << std::chrono::duration<double, std::chrono::milliseconds::period>(std::chrono::steady_clock::now() - before_compile).count() << "ms" << std::endl;
}

void Context::waitForCompletion()
{
    if (!commandBuffer || !fence)
    {
        return;
    }

    if (commands.empty() && buildAccelerationStructureCommands.empty())
    {
        return;
    }

    // we must wait for the queue to empty before we can safely clean up the commandBuffer
    uint64_t timeout = 1000000000;
    if (timeout > 0)
    {
        VkResult result;
        while ((result = fence->wait(timeout)) == VK_TIMEOUT)
        {
            std::cout << "Context::waitForCompletion() " << this << " fence->wait() timed out, trying again." << std::endl;
        }

        if (result != VK_SUCCESS)
        {
            std::cout << "Context::waitForCompletion()  " << this << " fence->wait() failed with error. VkResult = " << result << std::endl;
        }
    }

#if REPORT_STATS
    std::cout << "Context::waitForCompletion() copyBufferDataCommands = " << copyBufferDataCommands.size() << ", copyImageDataCommands = " << copyImageDataCommands.size() << ", commands = " << commands.size() << std::endl;
#endif

    commands.clear();
}
