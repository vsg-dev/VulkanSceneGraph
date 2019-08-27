/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/nodes/Commands.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <chrono>
#include <iostream>

using namespace vsg;

MemoryBufferPools::MemoryBufferPools(const std::string& in_name, Device* in_device, BufferPreferences preferences) :
    name(in_name),
    device(in_device),
    bufferPreferences(preferences)
{
}

BufferData MemoryBufferPools::reserveBufferData(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties)
{
    BufferData bufferData;
    for (auto& bufferFromPool : bufferPools)
    {
        if (!bufferFromPool->full() && bufferFromPool->usage() == bufferUsageFlags)
        {
            MemorySlots::OptionalOffset reservedBufferSlot = bufferFromPool->reserve(totalSize, alignment);
            if (reservedBufferSlot.first)
            {
                bufferData._buffer = bufferFromPool;
                bufferData._offset = reservedBufferSlot.second;
                bufferData._range = totalSize;

                // std::cout<<name<<" : MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") _offset = "<<bufferData._offset<<std::endl;

                return bufferData;
            }
        }
    }
    // std::cout<<name<<" : Failed to space in existing buffers with  MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") bufferPools.size() = "<<bufferPools.size()<<" looking to allocated new Buffer."<<std::endl;

    VkDeviceSize deviceSize = totalSize;

    VkDeviceSize minumumBufferSize = bufferPreferences.minimumBufferSize;
    if (deviceSize < minumumBufferSize)
    {
        deviceSize = minumumBufferSize;
    }

    bufferData._buffer = vsg::Buffer::create(device, deviceSize, bufferUsageFlags, sharingMode);

    MemorySlots::OptionalOffset reservedBufferSlot = bufferData._buffer->reserve(totalSize, alignment);
    bufferData._offset = reservedBufferSlot.second;
    bufferData._range = totalSize;

    // std::cout<<name<<" : Created new Buffer "<<bufferData._buffer.get()<<" totalSize "<<totalSize<<" deviceSize = "<<deviceSize<<std::endl;

    if (!bufferData._buffer->full())
    {
        // std::cout<<name<<"  inserting new Buffer into Context.bufferPools"<<std::endl;
        bufferPools.push_back(bufferData._buffer);
    }

    // std::cout<<name<<" : bufferData._offset = "<<bufferData._offset<<std::endl;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, *bufferData._buffer, &memRequirements);

    ref_ptr<DeviceMemory> deviceMemory;
    MemorySlots::OptionalOffset reservedMemorySlot(false, 0);

    for (auto& memoryFromPool : memoryPools)
    {
        if (!memoryFromPool->full() && memoryFromPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits)
        {
            reservedMemorySlot = memoryFromPool->reserve(deviceSize);
            if (reservedMemorySlot.first)
            {
                deviceMemory = memoryFromPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {
        VkDeviceSize minumumDeviceMemorySize = bufferPreferences.minimumBufferDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

        if (memRequirements.size < minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryProperties); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        if (deviceMemory)
        {
            reservedMemorySlot = deviceMemory->reserve(deviceSize);
            if (!deviceMemory->full())
            {
                memoryPools.push_back(deviceMemory);
            }
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            std::cout << name << " : DeviceMemory is full " << deviceMemory.get() << std::endl;
        }
    }

    if (!reservedMemorySlot.first)
    {
        // std::cout<<name<<" : Completely Failed to space for MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
        return BufferData();
    }

    // std::cout<<name<<" : Allocated new buffer, MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
    bufferData._buffer->bind(deviceMemory, reservedMemorySlot.second);

    return bufferData;
}

MemoryBufferPools::DeviceMemoryOffset MemoryBufferPools::reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryProperties, void* pNextAllocInfo)
{
    VkDeviceSize totalSize = memRequirements.size;

    ref_ptr<DeviceMemory> deviceMemory;
    MemorySlots::OptionalOffset reservedSlot(false, 0);

    for (auto& memoryPool : memoryPools)
    {
        if (!memoryPool->full() && memoryPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits)
        {
            reservedSlot = memoryPool->reserve(totalSize);
            if (reservedSlot.first)
            {
                deviceMemory = memoryPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {
        VkDeviceSize minumumDeviceMemorySize = bufferPreferences.minimumImageDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

        //std::cout<<"Creating new local DeviceMemory"<<std::endl;
        if (memRequirements.size < minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryProperties, pNextAllocInfo);
        if (deviceMemory)
        {
            reservedSlot = deviceMemory->reserve(totalSize);
            if (!deviceMemory->full())
            {
                //std::cout<<"  inserting DeviceMemory into memoryPool "<<deviceMemory.get()<<std::endl;
                memoryPools.push_back(deviceMemory);
            }
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            //std::cout<<"DeviceMemory is full "<<deviceMemory.get()<<std::endl;
        }
    }

    if (!reservedSlot.first)
    {
        std::cout << "Failed to reserve slot" << std::endl;
        return DeviceMemoryOffset();
    }

    //std::cout << "MemoryBufferPools::reserveMemory() allocated memory at " << reservedSlot.second << std::endl;
    return MemoryBufferPools::DeviceMemoryOffset(deviceMemory, reservedSlot.second);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::CopyAndReleaseBufferDataCommand
//
CopyAndReleaseBufferDataCommand::~CopyAndReleaseBufferDataCommand()
{
    source.release();
}

void CopyAndReleaseBufferDataCommand::dispatch(CommandBuffer& commandBuffer) const
{
    //std::cout<<"CopyAndReleaseBufferDataCommand::dispatch(CommandBuffer& commandBuffer) source._offset = "<<source._offset<<", "<<destination._offset<<std::endl;
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = source._offset;
    copyRegion.dstOffset = destination._offset;
    copyRegion.size = source._range;
    vkCmdCopyBuffer(commandBuffer, *source._buffer, *destination._buffer, 1, &copyRegion);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::CopyAndReleaseImageDataCommand
//
CopyAndReleaseImageDataCommand::~CopyAndReleaseImageDataCommand()
{
    source.release();
}

void CopyAndReleaseImageDataCommand::dispatch(CommandBuffer& commandBuffer) const
{
    ref_ptr<Buffer> imageStagingBuffer(source._buffer);
    ref_ptr<Data> data(source._data);
    ref_ptr<Image> textureImage(destination._imageView->getImage());
    ref_ptr<Sampler> sampler(destination._sampler);
    VkImageLayout targetImageLayout = destination._imageLayout;

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
            region.bufferOffset = source._offset + mipmapOffsets[mipLevel] * valueSize;
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
        region.bufferOffset = source._offset;
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
        // no mip maps required so just copy image without any extra processing.
        ImageMemoryBarrier preCopyImageMemoryBarrier(
            0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            textureImage);

        preCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkBufferImageCopy region = {};
        region.bufferOffset = source._offset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, depth};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        ImageMemoryBarrier postCopyImageMemoryBarrier(
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetImageLayout,
            textureImage);

        postCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Context
//
Context::Context(Device* in_device, BufferPreferences bufferPreferences) :
    device(in_device),
    deviceMemoryBufferPools("Device_MemoryBufferPool", device, bufferPreferences),
    stagingMemoryBufferPools("Staging_MemoryBufferPool", device, bufferPreferences)
{
}

ref_ptr<CommandBuffer> Context::getOrCreateCommandBuffer()
{
    if (!commandBuffer)
    {
        commandBuffer = vsg::CommandBuffer::create(device, commandPool, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    }

    return commandBuffer;
}

ref_ptr<Fence> Context::getOrCreateFence()
{
    if (!fence)
    {
        fence = vsg::Fence::create(device);
    }

    return fence;
}

void Context::dispatchCommands()
{
    if (commands.empty() && copyBufferDataCommands.empty() && copyImageDataCommands.empty()) return;

    auto before_compile = std::chrono::steady_clock::now();

    uint64_t timeout = 100000000000;
    getOrCreateCommandBuffer();
    getOrCreateFence();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = commandBuffer->flags();

    vkBeginCommandBuffer(*commandBuffer, &beginInfo);

    // issue commands of interest
    {
        for (auto& command : copyBufferDataCommands) command->dispatch(*commandBuffer);
        for (auto& command : copyImageDataCommands) command->dispatch(*commandBuffer);
        for (auto& command : commands) command->dispatch(*commandBuffer);
    }

    vkEndCommandBuffer(*commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer->data();

    // we must wait for the queue to empty before we can safely clean up the commandBuffer
    if (fence)
    {
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, *fence);
        if (timeout > 0)
        {
            VkResult result = fence->wait(timeout);
            if (result == VK_SUCCESS)
            { /*std::cout<<"Fence has successedully signaled"<<std::endl;*/
            }
            else
            {
                std::cout << "Fence failed to signal : " << result << std::endl;
                while ((result = fence->wait(timeout)) != VK_SUCCESS)
                {
                    std::cout << "   Fence failed again, trying another wait : " << result << std::endl;
                }
                std::cout << "   Finally we have success. " << result << std::endl;
            }

            fence->reset();
        }
    }
    else
    {
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
    }

    commands.clear();

    std::cout << "Context::dispatchCommands()  time " << std::chrono::duration<double, std::chrono::milliseconds::period>(std::chrono::steady_clock::now() - before_compile).count() << "ms" << std::endl;
    ;
}
