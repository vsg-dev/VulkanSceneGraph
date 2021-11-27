/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/commands/Commands.h>
#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Extensions.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// BuildAccelerationStructureCommand
//

BuildAccelerationStructureCommand::BuildAccelerationStructureCommand(Device* device, const VkAccelerationStructureBuildGeometryInfoKHR& info, const VkAccelerationStructureKHR& structure, const std::vector<uint32_t>& primitiveCounts, Allocator* allocator) :
    Inherit(allocator),
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
    Extensions* extensions = Extensions::Get(_device, true);
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

void BuildAccelerationStructureCommand::setScratchBuffer(ref_ptr<Buffer>& scratchBuffer)
{
    _scratchBuffer = scratchBuffer;
    Extensions* extensions = Extensions::Get(_device, true);
    VkBufferDeviceAddressInfo devAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, _scratchBuffer->vk(_device->deviceID)};
    _accelerationStructureInfo.scratchData.deviceAddress = extensions->vkGetBufferDeviceAddressKHR(_device->getDevice(), &devAddressInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Context
//
Context::Context(Device* in_device, const ResourceRequirements& resourceRequirements) :
    deviceID(in_device->deviceID),
    device(in_device),
    deviceMemoryBufferPools(MemoryBufferPools::create("Device_MemoryBufferPool", device, resourceRequirements)),
    stagingMemoryBufferPools(MemoryBufferPools::create("Staging_MemoryBufferPool", device, resourceRequirements)),
    scratchBufferSize(0)
{
    //semaphore = vsg::Semaphore::create(device);
    scratchMemory = ScratchMemory::create(4096);
}

Context::Context(const Context& context) :
    Inherit(context),
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

ShaderCompiler* Context::getOrCreateShaderCompiler()
{
    if (shaderCompiler) return shaderCompiler;

#ifdef HAS_GLSLANG
    shaderCompiler = ShaderCompiler::create();

    if (device && device->getInstance())
    {
        shaderCompiler->defaults->vulkanVersion = device->getInstance()->apiVersion;
    }

#endif

    return shaderCompiler;
}

void Context::copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest)
{
    if (!copyImageCmd)
    {
        copyImageCmd = CopyAndReleaseImage::create(stagingMemoryBufferPools);
        commands.push_back(copyImageCmd);
    }

    copyImageCmd->copy(data, dest);
}

void Context::copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels)
{
    if (!copyImageCmd)
    {
        copyImageCmd = CopyAndReleaseImage::create(stagingMemoryBufferPools);
        commands.push_back(copyImageCmd);
    }

    copyImageCmd->copy(data, dest, numMipMapLevels);
}

void Context::copy(ref_ptr<BufferInfo> src, ref_ptr<BufferInfo> dest)
{
    if (!copyBufferCmd)
    {
        copyBufferCmd = CopyAndReleaseBuffer::create();
        commands.emplace_back(copyBufferCmd);
    }

    copyBufferCmd->add(src, dest);
}

bool Context::record()
{
    if (commands.empty() && buildAccelerationStructureCommands.empty()) return false;

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

    // create scratch buffer and issue build acceleration structure commands
    ref_ptr<Buffer> scratchBuffer;
    ref_ptr<DeviceMemory> scratchBufferMemory;
    if (scratchBufferSize > 0)
    {
        scratchBuffer = vsg::createBufferAndMemory(device, scratchBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        for (auto& command : buildAccelerationStructureCommands)
        {
            command->setScratchBuffer(scratchBuffer);
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

    return true;
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

    VkResult result;
    while ((result = fence->wait(timeout)) == VK_TIMEOUT)
    {
        std::cout << "Context::waitForCompletion() " << this << " fence->wait() timed out, trying again." << std::endl;
    }

    if (result != VK_SUCCESS)
    {
        std::cout << "Context::waitForCompletion()  " << this << " fence->wait() failed with error. VkResult = " << result << std::endl;
    }

    commands.clear();
    copyImageCmd = nullptr;
    copyBufferCmd = nullptr;
}
