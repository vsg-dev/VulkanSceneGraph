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

using namespace vsg;

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
                                                  _instanceBuffer.valid() ? _instanceBuffer->vk(commandBuffer.deviceID) : (VkBuffer)VK_NULL_HANDLE,
                                                  0,
                                                  VK_FALSE,
                                                  _accelerationStructure,
                                                  VK_NULL_HANDLE,
                                                  _scratchBuffer->vk(commandBuffer.deviceID),
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

ShaderCompiler* Context::getOrCreateShaderCompiler()
{
    if (shaderCompiler) return shaderCompiler;

#ifdef HAS_GLSLANG
    shaderCompiler = new ShaderCompiler;
#endif

    return shaderCompiler;
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
        scratchBuffer = vsg::createBufferAndMemory(device, scratchBufferSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

    commands.clear();
}
