#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#include <vsg/utils/Instrumentation.h>

using namespace tracy;

namespace vsg
{

class TracyInstrumentation : public vsg::Inherit<vsg::Instrumentation, TracyInstrumentation>
{
public:

    TracyInstrumentation()
    {
    }

    mutable std::map<vsg::Device*, VkCtx*> ctxMap;
    mutable VkCtx* ctx = nullptr;

    uint32_t cpu_instumentation_level = 3;
    uint32_t gpu_instumentation_level = 3;

    void enterFrame(vsg::FrameStamp&) override {}

    void leaveFrame(vsg::FrameStamp&) override
    {
        FrameMark;
    }

    void enterCommandBuffer(vsg::CommandBuffer& commandBuffer) override
    {
        auto device = commandBuffer.getDevice();
        ctx = ctxMap[device];
        if (!ctx)
        {
            auto queue = device->getQueue(commandBuffer.getCommandPool()->queueFamilyIndex, 0);
            auto commandPool = vsg::CommandPool::create(device, queue->queueFamilyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            auto temporaryCommandBuffer = commandPool->allocate();
            ctx = ctxMap[device] = TracyVkContext(device->getPhysicalDevice()->vk(), device->vk(), queue->vk(), temporaryCommandBuffer->vk());
        }

        if (ctx)
        {
            TracyVkCollect(ctx, commandBuffer.vk());
        }
    }

    void leaveCommandBuffer() override
    {
        ctx = nullptr;
    }

    void enter(const vsg::SourceLocation* slcloc, uint64_t& reference) const override
    {
        if (!GetProfiler().IsConnected() || (slcloc->level > cpu_instumentation_level))
        {
            reference = 0;
            return;
        }

        reference = GetProfiler().ConnectionId();

        TracyQueuePrepare( QueueType::ZoneBegin );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, (uint64_t)slcloc );
        TracyQueueCommit( zoneBeginThread );
    }

    void leave(const vsg::SourceLocation*, uint64_t& reference) const override
    {
        if( reference==0 || GetProfiler().ConnectionId() != reference ) return;

        TracyQueuePrepare( QueueType::ZoneEnd );
        MemWrite( &item->zoneEnd.time, Profiler::GetTime() );
        TracyQueueCommit( zoneEndThread );
    }

    void enter(const vsg::SourceLocation* slcloc, uint64_t& reference, vsg::CommandBuffer& cmdbuf) const override
    {
        if (!GetProfiler().IsConnected() || (slcloc->level > gpu_instumentation_level))
        {
            reference = 0;
            return;
        }

        reference = GetProfiler().ConnectionId();

        const auto queryId = ctx->NextQueryId();
        CONTEXT_VK_FUNCTION_WRAPPER( vkCmdWriteTimestamp( cmdbuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, ctx->GetQueryPool(), queryId ) );

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::GpuZoneBeginSerial );
        MemWrite( &item->gpuZoneBegin.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)slcloc );
        MemWrite( &item->gpuZoneBegin.thread, GetThreadHandle() );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, ctx->GetId() );
        Profiler::QueueSerialFinish();
    }

    void leave(const vsg::SourceLocation* /*slcloc*/, uint64_t& reference, vsg::CommandBuffer& cmdbuf) const override
    {
        if( reference==0 || GetProfiler().ConnectionId() != reference ) return;

        const auto queryId = ctx->NextQueryId();
        CONTEXT_VK_FUNCTION_WRAPPER( vkCmdWriteTimestamp( cmdbuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, ctx->GetQueryPool(), queryId ) );

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::GpuZoneEndSerial );
        MemWrite( &item->gpuZoneEnd.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneEnd.thread, GetThreadHandle() );
        MemWrite( &item->gpuZoneEnd.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneEnd.context, ctx->GetId() );
        Profiler::QueueSerialFinish();
    }

protected:

    ~TracyInstrumentation()
    {
        for(auto itr = ctxMap.begin(); itr != ctxMap.end(); ++itr)
        {
            TracyVkDestroy(itr->second);
        }
    }
};

}
