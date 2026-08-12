/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/Commands.h>
#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/core/Version.h>
#include <vsg/io/Logger.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DynamicState.h>
#include <vsg/ui/UIEvent.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/DescriptorPools.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// BuildAccelerationStructureCommand
//

BuildAccelerationStructureCommand::BuildAccelerationStructureCommand(Device* device, const VkAccelerationStructureBuildGeometryInfoKHR& info, const VkAccelerationStructureKHR& structure, const std::vector<uint32_t>& primitiveCounts) :
    _device(device),
    _accelerationStructureInfo(info),
    _accelerationStructure(structure)
{
    _accelerationStructureInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    _accelerationStructureInfo.dstAccelerationStructure = _accelerationStructure;
    _accelerationStructureGeometries = std::vector<VkAccelerationStructureGeometryKHR>(_accelerationStructureInfo.pGeometries, _accelerationStructureInfo.pGeometries + _accelerationStructureInfo.geometryCount);
    _accelerationStructureInfo.pGeometries = _accelerationStructureGeometries.data();
    for (const auto c : primitiveCounts)
    {
        _accelerationStructureBuildRangeInfos.emplace_back();
        _accelerationStructureBuildRangeInfos.back().firstVertex = 0;
        _accelerationStructureBuildRangeInfos.back().primitiveCount = c;
        _accelerationStructureBuildRangeInfos.back().primitiveOffset = 0;
        _accelerationStructureBuildRangeInfos.back().transformOffset = 0;
    }
}

void BuildAccelerationStructureCommand::record(CommandBuffer& commandBuffer) const
{
    auto extensions = commandBuffer.getDevice()->getExtensions();
    const VkAccelerationStructureBuildRangeInfoKHR* rangeInfos = _accelerationStructureBuildRangeInfos.data();
    extensions->vkCmdBuildAccelerationStructuresKHR(
        commandBuffer,
        1,
        &_accelerationStructureInfo,
        &rangeInfos);

    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &memoryBarrier, 0, 0, 0, 0);
}

void BuildAccelerationStructureCommand::setScratchBuffer(ref_ptr<Buffer> scratchBuffer)
{
    _scratchBuffer = scratchBuffer;
    auto extensions = _device->getExtensions();
    VkBufferDeviceAddressInfo devAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, _scratchBuffer->vk(_device->deviceID)};
    _accelerationStructureInfo.scratchData.deviceAddress = extensions->vkGetBufferDeviceAddressKHR(*_device, &devAddressInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Context
//
Context::Context(Device* in_device, const ResourceRequirements& in_resourceRequirements) :
    deviceID(in_device->deviceID),
    device(in_device),
    resourceRequirements(in_resourceRequirements),
    scratchBufferSize(0)
{
    //semaphore = vsg::Semaphore::create(device);
    scratchMemory = ScratchMemory::create(4096);

    vsg::debug("Context::Context() ", this);

    deviceMemoryBufferPools = device->deviceMemoryBufferPools.ref_ptr();
    if (!deviceMemoryBufferPools)
    {
        device->deviceMemoryBufferPools = deviceMemoryBufferPools = MemoryBufferPools::create("Device_MemoryBufferPool", device, in_resourceRequirements);
        vsg::debug("Context::Context() creating new deviceMemoryBufferPools = ", deviceMemoryBufferPools);
    }
    else
    {
        vsg::debug("Context::Context() reusing deviceMemoryBufferPools = ", deviceMemoryBufferPools);
    }

    stagingMemoryBufferPools = device->stagingMemoryBufferPools.ref_ptr();
    if (!stagingMemoryBufferPools)
    {
        device->stagingMemoryBufferPools = stagingMemoryBufferPools = MemoryBufferPools::create("Staging_MemoryBufferPool", device, in_resourceRequirements);
        vsg::debug("Context::Context() creating new stagingMemoryBufferPools = ", stagingMemoryBufferPools);
    }
    else
    {
        vsg::debug("Context::Context() reusing stagingMemoryBufferPools = ", stagingMemoryBufferPools);
    }

    descriptorPools = device->descriptorPools.ref_ptr();
    if (!descriptorPools)
    {
        device->descriptorPools = descriptorPools = DescriptorPools::create(device);
        vsg::debug("Context::Context() creating new descriptorPools = ", descriptorPools);
    }
    else
    {
        vsg::debug("Context::Context() reusing descriptorPools = ", descriptorPools);
    }

    if ((resourceRequirements.viewportStateHint & DYNAMIC_VIEWPORTSTATE))
    {
        defaultPipelineStates.push_back(DynamicState::create(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR));
    }
}

Context::Context(const Context& context) :
    Inherit(context),
    deviceID(context.deviceID),
    device(context.device),
    view(context.view),
    viewID(context.viewID),
    mask(context.mask),
    viewDependentState(context.viewDependentState),
    renderPass(context.renderPass),
    defaultPipelineStates(context.defaultPipelineStates),
    overridePipelineStates(context.overridePipelineStates),
    descriptorPools(context.descriptorPools),
    graphicsQueue(context.graphicsQueue),
    commandPool(context.commandPool),
    deviceMemoryBufferPools(context.deviceMemoryBufferPools),
    stagingMemoryBufferPools(context.stagingMemoryBufferPools),
    transferTask(context.transferTask),
    scratchBufferSize(context.scratchBufferSize)
{
    scratchMemory = ScratchMemory::create(4096);
}

Context::~Context()
{
    if (requiresWaitForCompletion)
    {
        waitForCompletion();
    }
}

void Context::reset()
{
    if ((commands.size()>0) || bufferInfosToCopy.size()>0 || copyImageCmd || copyBufferCmd)
    {
        vsg::info("Context::reset() commands.size() = ", commands.size(), ", bufferInfosToCopy.size() = ", bufferInfosToCopy.size(), ", copyImageCmd = ", copyBufferCmd, ", copyBufferCmd= ", copyBufferCmd);
    }

    commands.clear();

    bufferInfosToCopy.clear();

    copyImageCmd.reset();
    copyBufferCmd.reset();
}


ref_ptr<CommandBuffer> Context::getOrCreateCommandBuffer()
{
    if (!commandBuffer)
    {
        commandBuffer = commandPool->allocate();
    }

    return commandBuffer;
}

ShaderCompiler* Context::getOrCreateShaderCompiler()
{
    if (shaderCompiler) return shaderCompiler;

#if VSG_SUPPORTS_ShaderCompiler
    shaderCompiler = ShaderCompiler::create();

    if (device && device->getInstance())
    {
        shaderCompiler->defaults->vulkanVersion = device->getInstance()->apiVersion;
    }

#endif

    return shaderCompiler;
}

VkResult Context::reserve(ResourceRequirements& requirements)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "Context reserve", COLOR_COMPILE)

    VkResult result = VK_SUCCESS;

    if (deviceMemoryBufferPools->compileTraversalUseReserve) result = deviceMemoryBufferPools->reserve(requirements);

    resourceRequirements.maxSlots.update(requirements.maxSlots);

    descriptorPools->reserve(requirements);

    return result;
}

ref_ptr<DescriptorSet::Implementation> Context::allocateDescriptorSet(DescriptorSetLayout* descriptorSetLayout)
{
    return descriptorPools->allocateDescriptorSet(descriptorSetLayout);
}

void Context::copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "Context copy", COLOR_COMPILE)

    // info("Context::copy(", data, ", ", dest, ") ", this, ", ", transferTask);

    if (!copyImageCmd)
    {
        copyImageCmd = CopyAndReleaseImage::create(stagingMemoryBufferPools);
        commands.push_back(copyImageCmd);
    }

    copyImageCmd->copy(data, dest);
}

void Context::copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "Context copy", COLOR_COMPILE)

    // info("Context::copy(", data, ", ", dest, ") ", this, ", ", transferTask);

    if (!copyImageCmd)
    {
        copyImageCmd = CopyAndReleaseImage::create(stagingMemoryBufferPools);
        commands.push_back(copyImageCmd);
    }

    copyImageCmd->copy(data, dest, numMipMapLevels);
}

void Context::copy(ref_ptr<BufferInfo> src, ref_ptr<BufferInfo> dest)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "Context copy", COLOR_COMPILE)

    // info("Context::copy(", src, ", ", dest, ") ", this, ", ", transferTask);

    if (!copyBufferCmd)
    {
        copyBufferCmd = CopyAndReleaseBuffer::create();
        commands.emplace_back(copyBufferCmd);
    }

    copyBufferCmd->add(src, dest);
}

bool Context::record()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Context record", COLOR_COMPILE)

    // vsg::info("Context::record() ", this, ", commands.size() = ", commands.size(), ", bufferInfosToCopy.size() = ", bufferInfosToCopy.size(), ", imageInfosToCopy.size() = ", imageInfosToCopy.size());

    if (transferTask)
    {
        if (!bufferInfosToCopy.empty())
        {
            transferTask->assign(bufferInfosToCopy);
            bufferInfosToCopy.clear();
        }
        if (!imageInfosToCopy.empty())
        {
            transferTask->assign(imageInfosToCopy);
            imageInfosToCopy.clear();
        }
    }

    if (commands.empty() && buildAccelerationStructureCommands.empty()) return false;

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

    {
        COMMAND_BUFFER_INSTRUMENTATION(instrumentation, *commandBuffer, "Context record", COLOR_COMPILE)

        if ((bufferInfosToCopy.size()+imageInfosToCopy.size())>0) vsg::warn("Context::record() ", this, " Need to implement copying of data.");


        // issue commands of interest
        {
            for (auto& command : commands) command->record(*commandBuffer);
        }

        // create scratch buffer and issue build acceleration structure commands
        if (scratchBufferSize > 0)
        {
            VkMemoryAllocateFlagsInfo memFlags = {};
            memFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            memFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            ref_ptr<Buffer> scratchBuffer = vsg::createBufferAndMemory(device, scratchBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memFlags);

            for (auto& command : buildAccelerationStructureCommands)
            {
                command->setScratchBuffer(scratchBuffer);
                command->record(*commandBuffer);
            }
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
        vsg::info("Context::record() semaphore assigned to submitInfo ", semaphore);

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

    requiresWaitForCompletion = true;

    return true;
}

void Context::waitForCompletion()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Context waitForCompletion", COLOR_COMPILE)

    vsg::info("Context::waitForCompletion() ", this);

    if (!requiresWaitForCompletion || !commandBuffer || !fence)
    {
        return;
    }

    // auto start_point = vsg::clock::now();

    // we must wait for the queue to empty before we can safely clean up the commandBuffer
    uint64_t timeout = 1000000000;

    VkResult result;
    while ((result = fence->wait(timeout)) == VK_TIMEOUT)
    {
        info("Context::waitForCompletion() ", this, " fence->wait() timed out, trying again.");
    }

    if (result != VK_SUCCESS)
    {
        info("Context::waitForCompletion()  ", this, " fence->wait() failed with error. VkResult = ", result);
    }

    //vsg::info("Context::waitForCompletion() ", std::chrono::duration<double, std::chrono::milliseconds::period>(vsg::clock::now() - start_point).count());

    requiresWaitForCompletion = false;
    commands.clear();
    copyImageCmd = nullptr;
    copyBufferCmd = nullptr;
}

bool Context::createBufferAndTransferData(const BufferInfoList& bufferInfoList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    if (bufferInfoList.empty()) return false;

    VkDeviceSize alignment = 4;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;
    else if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        alignment = device->getPhysicalDevice()->getProperties().limits.minStorageBufferOffsetAlignment;

#if 1
    if (VkResult result = deviceMemoryBufferPools->reserve(bufferInfoList, alignment, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sharingMode, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); result == VK_SUCCESS)
    {
#if 1
        bufferInfosToCopy.reserve(bufferInfosToCopy.size() + bufferInfoList.size());
        bufferInfosToCopy.insert(bufferInfosToCopy.end(), bufferInfoList.begin(), bufferInfoList.end());
#else
        if (transferTask) transferTask->assign(bufferInfoList);
        else warn("vsg::createBufferAndTransferData() transfer task missing, need to implement transfer.");
#endif
        return true;
    }
    else
    {
        return false;
    }
#else

    // info("vsg::createBufferAndTransferData(const BufferInfoList& bufferInfoList, VkBufferUsageFlags usage, VkSharingMode sharingMode) usage = ", usage, ", alignment = ", alignment);

    //transferTask = nullptr;

    ref_ptr<BufferInfo> deviceBufferInfo;
    size_t numBuffersRequired = 0;
    bool containsMultipleParents = false;
    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->data)
        {
            if (bufferInfo->data->getModifiedCount(bufferInfo->copiedModifiedCounts[deviceID]))
            {
                ++numBuffersRequired;
            }
        }

        if (bufferInfo->parent)
        {
            if (deviceBufferInfo && bufferInfo->parent != deviceBufferInfo) containsMultipleParents = true;
            deviceBufferInfo = bufferInfo->parent;
        }
    }

    if (numBuffersRequired == 0)
    {
        debug("\nvsg::createBufferAndTransferData(...) already all compiled. deviceID = ", deviceID);
        return false;
    }

    if (containsMultipleParents)
    {
        warn("vsg::createBufferAndTransferData(...) does not support multiple parent BufferInfo.");
        return false;
    }

    VkDeviceSize totalSize = 0;
    VkDeviceSize offset = 0;
    for (const auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->data)
        {
            bufferInfo->offset = offset;
            bufferInfo->range = bufferInfo->data->dataSize();
            VkDeviceSize endOfEntry = offset + bufferInfo->range;
            offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
            //info("  BufferInfo.data = ", bufferInfo->data, ", dynamic = ", bufferInfo->data->getLayout().dynamic);
        }
    }

    totalSize = offset;
    if (totalSize == 0) return false;

    if (deviceBufferInfo && deviceBufferInfo->buffer)
    {
        if (totalSize != deviceBufferInfo->range)
        {
            warn("Existing deviceBufferInfo, ", deviceBufferInfo, ", deviceBufferInfo->range  = ", deviceBufferInfo->range, ", ", totalSize, " NOT compatible");
            return false;
        }
        else
        {
            debug("Existing deviceBufferInfo, ", deviceBufferInfo, ", deviceBufferInfo->range  = ", deviceBufferInfo->range, ", ", totalSize, " with compatible size");

            // make sure the VkBuffer is created
            deviceBufferInfo->buffer->compile(*this);

            if (!deviceBufferInfo->buffer->getDeviceMemory(deviceID))
            {
                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(*device, deviceBufferInfo->buffer->vk(device->deviceID), &memRequirements);

                auto deviceMemoryOffset = deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                if (deviceMemoryOffset.first)
                    deviceBufferInfo->buffer->bind(deviceMemoryOffset.first, deviceMemoryOffset.second);
                else
                {
                    debug("vsg::createBufferAndTransferData() Failure to assign memory to existing BufferInfo");
                    return false;
                }
            }
        }
    }

    if (!deviceBufferInfo)
    {
        VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
        deviceBufferInfo = deviceMemoryBufferPools->reserveBuffer(totalSize, alignment, bufferUsageFlags, sharingMode, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    if (!deviceBufferInfo)
    {
        debug("vsg::createBufferAndTransferData() Failure to assign Buffer");
        return false;
    }

    debug("deviceBufferInfo->buffer ", deviceBufferInfo->buffer, ", ", deviceBufferInfo->offset, ", ", deviceBufferInfo->range, ")");

    // assign the buffer to the bufferData entries and shift the offsets to offset within the buffer
    for (const auto& bufferInfo : bufferInfoList)
    {
        bufferInfo->buffer = deviceBufferInfo->buffer;
        bufferInfo->offset += deviceBufferInfo->offset;
    }

    if (transferTask)
    {
        vsg::debug("vsg::createBufferAndTransferData(..)");

        for (auto& bufferInfo : bufferInfoList)
        {
            vsg::debug("    ", bufferInfo, ", ", bufferInfo->data, ", ", bufferInfo->buffer, ", ", bufferInfo->offset);
            bufferInfo->data->dirty();
            bufferInfo->parent = deviceBufferInfo;
        }

        transferTask->assign(bufferInfoList);

        return true;
    }

    auto stagingBufferInfo = stagingMemoryBufferPools->reserveBuffer(totalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    debug("stagingBufferInfo->buffer ", stagingBufferInfo->buffer.get(), ", ", stagingBufferInfo->offset, ", ", stagingBufferInfo->range, ")");

    ref_ptr<Buffer> stagingBuffer(stagingBufferInfo->buffer);
    ref_ptr<DeviceMemory> stagingMemory(stagingBuffer->getDeviceMemory(deviceID));

    if (!stagingMemory)
    {
        return false;
    }

    void* buffer_data;
    stagingMemory->map(stagingBuffer->getMemoryOffset(deviceID) + stagingBufferInfo->offset, stagingBufferInfo->range, 0, &buffer_data);
    char* ptr = reinterpret_cast<char*>(buffer_data);

    debug("    buffer_data ", buffer_data, ", stagingBufferInfo->offset=", stagingBufferInfo->offset, ", ", totalSize);

    for (const auto& bufferInfo : bufferInfoList)
    {
        const Data* data = bufferInfo->data;
        if (data)
        {
            std::memcpy(ptr + bufferInfo->offset - deviceBufferInfo->offset, data->dataPointer(), data->dataSize());
            if (data->properties.dataVariance == STATIC_DATA_UNREF_AFTER_TRANSFER)
            {
                bufferInfo->data.reset();
            }
        }
        bufferInfo->parent = deviceBufferInfo;
    }

    stagingMemory->unmap();

    copy(stagingBufferInfo, deviceBufferInfo);

    return true;
#endif
}

bool Context::copy(const ImageInfoList& imageInfoList)
{
    if (imageInfoList.empty()) return false;

    imageInfosToCopy.reserve(imageInfosToCopy.size() + imageInfoList.size());
    imageInfosToCopy.insert(imageInfosToCopy.end(), imageInfoList.begin(), imageInfoList.end());

    return true;
}
