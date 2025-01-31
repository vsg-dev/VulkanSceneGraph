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

    defaultPipelineStates.push_back(DynamicState::create());
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

void Context::reserve(const ResourceRequirements& requirements)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "Context reserve", COLOR_COMPILE)

    if (requirements.maxSlot > resourceRequirements.maxSlot)
    {
        resourceRequirements.maxSlot = requirements.maxSlot;
    }

    descriptorPools->reserve(requirements);
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

        // issue commands of interest
        {
            for (auto& command : commands) command->record(*commandBuffer);
        }

        // create scratch buffer and issue build acceleration structure commands
        if (scratchBufferSize > 0)
        {
            ref_ptr<Buffer> scratchBuffer = vsg::createBufferAndMemory(device, scratchBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

    //vsg::info("Conext::waitForCompletion() ", std::chrono::duration<double, std::chrono::milliseconds::period>(vsg::clock::now() - start_point).count());

    requiresWaitForCompletion = false;
    commands.clear();
    copyImageCmd = nullptr;
    copyBufferCmd = nullptr;
}
