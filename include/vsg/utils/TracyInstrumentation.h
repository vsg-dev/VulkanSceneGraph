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

    class TracySettings : public Inherit<Object, TracySettings>
    {
    public:
        uint32_t cpu_instrumentation_level = 3;
        uint32_t gpu_instrumentation_level = 3;
    };
    VSG_type_name(vsg::TracySettings);

#ifdef TRACY_ENABLE
    /// thread safe helper class for creating the Tracy VkCtx objects per Device.
    class TracyContexts : public Inherit<Object, TracyContexts>
    {
    public:
        VkCtx* getOrCreateContext(CommandBuffer& commandBuffer) const
        {
            std::scoped_lock<std::mutex> lock(mutex);

            ref_ptr<Device> device(commandBuffer.getDevice());
            auto& [ctx, requiresCollection] = ctxMap[device];
            if (!ctx)
            {
                auto queue = device->getQueue(commandBuffer.getCommandPool()->queueFamilyIndex, 0);
                auto commandPool = CommandPool::create(device, queue->queueFamilyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
                auto temporaryCommandBuffer = commandPool->allocate();
                auto extensions = device->getInstance()->getExtensions();

                if (device->supportsDeviceExtension("VK_EXT_calibrated_timestamps"))
                {
                    ctx = TracyVkContextCalibrated(device->getPhysicalDevice()->vk(), device->vk(), queue->vk(), temporaryCommandBuffer->vk(),
                                                   extensions->vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, extensions->vkGetCalibratedTimestampsEXT);
                }
                else
                {
                    ctx = TracyVkContext(device->getPhysicalDevice()->vk(), device->vk(), queue->vk(), temporaryCommandBuffer->vk());
                }
                requiresCollection = false;
            }

            if (ctx && requiresCollection)
            {
                TracyVkCollect(ctx, commandBuffer.vk());
                requiresCollection = false;
            }

            return ctx;
        }

        void frameComplete()
        {
            std::scoped_lock<std::mutex> lock(mutex);
            for (auto itr = ctxMap.begin(); itr != ctxMap.end(); ++itr)
            {
                itr->second.second = true;
            }
        }

        mutable std::mutex mutex;
        mutable std::map<ref_ptr<Device>, std::pair<VkCtx*, bool>> ctxMap;

    protected:
        ~TracyContexts()
        {
            for (auto itr = ctxMap.begin(); itr != ctxMap.end(); ++itr)
            {
                TracyVkDestroy(itr->second.first);
            }
        }
    };
    VSG_type_name(vsg::TracyContexts);

    /// TracyInstrumentation provides integration between the Instrumentation system and the Tracy profiler
    class TracyInstrumentation : public Inherit<Instrumentation, TracyInstrumentation>
    {
    public:
        TracyInstrumentation() :
            settings(TracySettings::create()),
            contexts(TracyContexts::create())
        {
        }

        TracyInstrumentation(TracyInstrumentation& parent) :
            settings(parent.settings),
            contexts(parent.contexts)
        {
        }

        ref_ptr<TracySettings> settings;
        ref_ptr<TracyContexts> contexts;
        mutable VkCtx* ctx = nullptr;
        bool requiresCollection = false;

        ref_ptr<Instrumentation> shareOrDuplicateForThreadSafety() override
        {
            return TracyInstrumentation::create(*this);
        }

        void setThreadName(const std::string& name) const override
        {
            tracy::SetThreadName(name.c_str());
        }

        void enterFrame(const SourceLocation*, uint64_t&, FrameStamp&) const override {}

        void leaveFrame(const SourceLocation*, uint64_t&, FrameStamp&) const override
        {
            contexts->frameComplete();

            FrameMark;
        }

        void enter(const SourceLocation* slcloc, uint64_t& reference, const Object*) const override
        {
#    ifdef TRACY_ON_DEMAND
            if (!GetProfiler().IsConnected() || (slcloc->level > settings->cpu_instrumentation_level))
#    else
            if (slcloc->level > settings->cpu_instrumentation_level)
#    endif
            {
                reference = 0;
                return;
            }

#    ifdef TRACY_ON_DEMAND
            reference = GetProfiler().ConnectionId();
#    else
            reference = 1;
#    endif

            TracyQueuePrepare(QueueType::ZoneBegin);
            MemWrite(&item->zoneBegin.time, tracy::Profiler::GetTime());
            MemWrite(&item->zoneBegin.srcloc, (uint64_t)slcloc);
            TracyQueueCommit(zoneBeginThread);
        }

        void leave(const SourceLocation*, uint64_t& reference, const Object*) const override
        {
#    ifdef TRACY_ON_DEMAND
            if (reference == 0 || GetProfiler().ConnectionId() != reference) return;
#    else
            if (reference == 0) return;
#    endif

            TracyQueuePrepare(QueueType::ZoneEnd);
            MemWrite(&item->zoneEnd.time, tracy::Profiler::GetTime());
            TracyQueueCommit(zoneEndThread);
        }

        void enterCommandBuffer(const SourceLocation* slcloc, uint64_t& reference, CommandBuffer& commandBuffer) const override
        {
            if (ctx = contexts->getOrCreateContext(commandBuffer))
            {
                enter(slcloc, reference, commandBuffer, nullptr);
            }
        }

        void leaveCommandBuffer(const SourceLocation* slcloc, uint64_t& reference, CommandBuffer& commandBuffer) const override
        {
            if (ctx)
            {
                leave(slcloc, reference, commandBuffer, nullptr);
            }

            ctx = nullptr;
        }

        void enter(const SourceLocation* slcloc, uint64_t& reference, CommandBuffer& cmdbuf, const Object*) const override
        {
#    ifdef TRACY_ON_DEMAND
            if (!ctx || !GetProfiler().IsConnected() || (slcloc->level > settings->gpu_instrumentation_level))
#    else
            if (!ctx || slcloc->level > settings->gpu_instrumentation_level)
#    endif
            {
                reference = 0;
                return;
            }

#    ifdef TRACY_ON_DEMAND
            reference = GetProfiler().ConnectionId();
#    else
            reference = 1;
#    endif

            const auto queryId = ctx->NextQueryId();
            CONTEXT_VK_FUNCTION_WRAPPER(vkCmdWriteTimestamp(cmdbuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, ctx->GetQueryPool(), queryId));

            auto item = tracy::Profiler::QueueSerial();
            MemWrite(&item->hdr.type, QueueType::GpuZoneBeginSerial);
            MemWrite(&item->gpuZoneBegin.cpuTime, tracy::Profiler::GetTime());
            MemWrite(&item->gpuZoneBegin.srcloc, (uint64_t)slcloc);
            MemWrite(&item->gpuZoneBegin.thread, GetThreadHandle());
            MemWrite(&item->gpuZoneBegin.queryId, uint16_t(queryId));
            MemWrite(&item->gpuZoneBegin.context, ctx->GetId());
            tracy::Profiler::QueueSerialFinish();
        }

        void leave(const SourceLocation*, uint64_t& reference, CommandBuffer& cmdbuf, const Object*) const override
        {
#    ifdef TRACY_ON_DEMAND
            if (reference == 0 || GetProfiler().ConnectionId() != reference) return;
#    else
            if (reference == 0) return;
#    endif

            const auto queryId = ctx->NextQueryId();
            CONTEXT_VK_FUNCTION_WRAPPER(vkCmdWriteTimestamp(cmdbuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, ctx->GetQueryPool(), queryId));

            auto item = tracy::Profiler::QueueSerial();
            MemWrite(&item->hdr.type, QueueType::GpuZoneEndSerial);
            MemWrite(&item->gpuZoneEnd.cpuTime, tracy::Profiler::GetTime());
            MemWrite(&item->gpuZoneEnd.thread, GetThreadHandle());
            MemWrite(&item->gpuZoneEnd.queryId, uint16_t(queryId));
            MemWrite(&item->gpuZoneEnd.context, ctx->GetId());
            tracy::Profiler::QueueSerialFinish();
        }
    };
    VSG_type_name(vsg::TracyInstrumentation);
#else
    class TracyInstrumentation : public Inherit<Instrumentation, TracyInstrumentation>
    {
    public:
        TracyInstrumentation()
        {
            vsg::info("TracyInstrumentation not supported and the tracy's TRACY_ENABLE is set to OFF.");
        }

        ref_ptr<TracySettings> settings;
    };
    VSG_type_name(vsg::TracyInstrumentation);
#endif
} // namespace vsg
