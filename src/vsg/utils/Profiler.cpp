/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/state/QueryPool.h>
#include <vsg/ui/FrameStamp.h>
#include <vsg/utils/Profiler.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ProfileLog
//
ProfileLog::ProfileLog(size_t size) :
    // TODO make user definable
    entries(size)
{
}

void ProfileLog::read(Input& input)
{
    entries.resize(input.readValue<uint64_t>("entries"));
}

void ProfileLog::write(Output& output) const
{
    output.writeValue<uint64_t>("entries", entries.size());
}

void ProfileLog::report(std::ostream& out)
{
    out << "frames " << frameIndices.size() << std::endl;
    for (auto frameIndex : frameIndices)
    {
        report(out, frameIndex);
        out << std::endl;
    }
}

uint64_t ProfileLog::report(std::ostream& out, uint64_t reference)
{
    indentation indent;
    out << "ProfileLog::report(" << reference << ")" << std::endl;
    out << "{" << std::endl;
    uint32_t tab = 1;
    indent += tab;

    static const char* typeNames[] = {
        "NO_TYPE",
        "FRAME",
        "CPU",
        "COMMAND_BUFFER",
        "GPU"};

    uint64_t startReference = reference;
    uint64_t endReference = entry(reference).reference;

    if (startReference > endReference)
    {
        std::swap(startReference, endReference);
    }

    for (uint64_t i = startReference; i <= endReference; ++i)
    {
        auto& first = entry(i);
        auto& second = entry(first.reference);
        auto cpu_duration = std::abs(std::chrono::duration<double, std::chrono::milliseconds::period>(second.cpuTime - first.cpuTime).count());

        auto gpu_duration = 0.0;
        if (first.gpuTime != 0 && second.gpuTime != 0)
        {
            gpu_duration = static_cast<double>((first.gpuTime < second.gpuTime) ? (second.gpuTime - first.gpuTime) : (first.gpuTime - second.gpuTime)) * timestampScaleToMilliseconds;
        }

        if (first.enter && first.reference == i + 1)
        {
            ++i;

            out << indent << "{ " << typeNames[first.type] << ", cpu_duration = " << cpu_duration << "ms, ";
            if (gpu_duration != 0.0) out << ", gpu_duration = " << gpu_duration << "ms, ";

            auto itr = threadNames.find(first.thread_id);
            if (itr != threadNames.end()) out << ", thread = " << itr->second;

            if (first.sourceLocation) out /*<<", file="<<first.sourceLocation->file*/ << ", func=" << first.sourceLocation->function << ", line=" << first.sourceLocation->line;
            // if (first.object) out<<", "<<first.object->className();
            out << " }" << std::endl;
        }
        else
        {
            if (first.enter)
                out << indent << "{ ";
            else
            {
                indent -= tab;
                out << indent << "} ";
            }

            out << typeNames[first.type] << ", cpu_duration = " << cpu_duration << "ms, ";
            if (gpu_duration != 0.0) out << ", gpu_duration = " << gpu_duration << "ms, ";

            auto itr = threadNames.find(first.thread_id);
            if (itr != threadNames.end()) out << ", thread = " << itr->second;

            if (first.sourceLocation) out /*<<", file="<<first.sourceLocation->file*/ << ", func=" << first.sourceLocation->function << ", line=" << first.sourceLocation->line;
            // if (first.object) out<<", "<<first.object->className();

            out << std::endl;

            if (first.enter) indent += tab;
        }
    }

    out << "}" << std::endl;

    return endReference + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GPUStatsCollection
//
void GPUStatsCollection::writeGpuTimestamp(CommandBuffer& commandBuffer, uint64_t reference, VkPipelineStageFlagBits pipelineStage)
{
    auto index = queryIndex.fetch_add(1);
    if (index < timestamps.size())
    {
        timestamps[index] = 0;
        references[index] = reference;
        return vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool->vk(), index);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Profiler
//
Profiler::Profiler(ref_ptr<Settings> in_settings) :
    settings(in_settings.valid() ? in_settings : Settings::create()),
    log(ProfileLog::create(settings->log_size)),
    perFrameGPUStats(3)
{
}


VkResult Profiler::getGpuResults(FrameStatsCollection& frameStats) const
{
    VkResult result = VK_SUCCESS;

    for (auto& gpuStats : frameStats.gpuStats)
    {
        if (gpuStats && gpuStats->queryIndex.load() > 0)
        {
            VkQueryResultFlags resultsFlags = /*VK_QUERY_RESULT_WAIT_BIT | */VK_QUERY_RESULT_64_BIT;
            auto count = std::min(static_cast<uint32_t>(gpuStats->timestamps.size()), gpuStats->queryIndex.load());
            result = vkGetQueryPoolResults(gpuStats->device->vk(), gpuStats->queryPool->vk(), 0, count, count * sizeof(uint64_t), gpuStats->timestamps.data(), sizeof(uint64_t), resultsFlags);
            if (result == VK_SUCCESS)
            {
                for (uint32_t i = 0; i < count; ++i)
                {
                    auto& gpu_entry = log->entry(gpuStats->references[i]);
                    gpu_entry.gpuTime = gpuStats->timestamps[i];
                }
                gpuStats->queryIndex = 0;
            }
            else
            {
                info("Profiler::getGpuResults() ", gpuStats, ", query failed with result = ", result);
            }

        }
    }

    return result;
}

void Profiler::setThreadName(const std::string& name) const
{
    log->threadNames[std::this_thread::get_id()] = name;
}

void Profiler::enterFrame(const SourceLocation* sl, uint64_t& reference, FrameStamp& frameStamp) const
{
    auto& entry = log->enter(reference, ProfileLog::FRAME);
    entry.sourceLocation = sl;
    entry.object = &frameStamp;

    getGpuResults(perFrameGPUStats[frameIndex]);
}

void Profiler::leaveFrame(const SourceLocation* sl, uint64_t& reference, FrameStamp& frameStamp) const
{
    uint64_t startReference = reference;
    auto& entry = log->leave(reference, ProfileLog::FRAME);
    entry.sourceLocation = sl;
    entry.object = &frameStamp;
    uint64_t endReference = reference;

    if (endReference >= static_cast<uint64_t>(log->entries.size()))
    {
        uint64_t safeReference = endReference - static_cast<uint64_t>(log->entries.size());
        size_t i = 0;
        for (; i < log->frameIndices.size(); ++i)
        {
            if (log->frameIndices[i] > safeReference)
            {
                break;
            }
        }
        if (i > 0)
        {
            log->frameIndices.erase(log->frameIndices.begin(), log->frameIndices.begin() + i);
        }
    }

    log->frameIndices.push_back(startReference);

    // advance the frame index to the next frame position in the perFrameGPUStats container
    ++frameIndex;
    if (frameIndex >= perFrameGPUStats.size()) frameIndex = 0;
}

void Profiler::enter(const SourceLocation* sl, uint64_t& reference, const Object* object) const
{
    if (settings->cpu_instrumentation_level >= sl->level)
    {
        auto& entry = log->enter(reference, ProfileLog::CPU);
        entry.sourceLocation = sl;
        entry.object = object;
    }
}

void Profiler::leave(const SourceLocation* sl, uint64_t& reference, const Object* object) const
{
    if (settings->cpu_instrumentation_level >= sl->level)
    {
        auto& entry = log->leave(reference, ProfileLog::CPU);
        entry.sourceLocation = sl;
        entry.object = object;
    }
}

void Profiler::enterCommandBuffer(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const
{
    if (settings->gpu_instrumentation_level >= sl->level)
    {
        auto& frameStats = perFrameGPUStats[frameIndex];

        uint32_t numQueries = settings->gpu_timestamp_size;

        auto& gpuStats = commandBuffer.gpuStats;


        if (!gpuStats)
        {
            auto physicalDevice = commandBuffer.getDevice()->getPhysicalDevice();
            const auto& limits = physicalDevice->getProperties().limits;

            if (limits.timestampComputeAndGraphics)
            {
                // limits.timestampPeriod is in nanoseconds

                log->timestampScaleToMilliseconds = 1e-6 * static_cast<double>(limits.timestampPeriod);

                gpuStats = GPUStatsCollection::create();

                gpuStats->device = commandBuffer.getDevice();
                gpuStats->queryPool = QueryPool::create(commandBuffer.getDevice(), VkQueryPoolCreateFlags{0}, VK_QUERY_TYPE_TIMESTAMP, numQueries, VkQueryPipelineStatisticFlags{0});
                gpuStats->references.resize(numQueries);
                gpuStats->timestamps.resize(numQueries);

                frameStats.gpuStats.push_back(gpuStats);
            }
            else
            {
                warn("Timestamps not supported by device.");
            }
        }

        auto& entry = log->enter(reference, ProfileLog::COMMAND_BUFFER);
        entry.sourceLocation = sl;
        entry.object = &commandBuffer;

        if (gpuStats)
        {
            vkCmdResetQueryPool(commandBuffer, gpuStats->queryPool->vk(), 0, numQueries);
            gpuStats->queryIndex = 0;

            gpuStats->writeGpuTimestamp(commandBuffer, reference, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }
    }
}

void Profiler::leaveCommandBuffer(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const
{
    if (settings->gpu_instrumentation_level >= sl->level)
    {
        auto& entry = log->leave(reference, ProfileLog::COMMAND_BUFFER);
        entry.sourceLocation = sl;
        entry.object = &commandBuffer;

        if (commandBuffer.gpuStats) commandBuffer.gpuStats->writeGpuTimestamp(commandBuffer, reference, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
}

void Profiler::enter(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer, const Object* object) const
{
    if (settings->gpu_instrumentation_level >= sl->level)
    {
        auto& entry = log->enter(reference, ProfileLog::GPU);
        entry.sourceLocation = sl;
        entry.object = object;

        if (commandBuffer.gpuStats) commandBuffer.gpuStats->writeGpuTimestamp(commandBuffer, reference, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
}

void Profiler::leave(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer, const Object* object) const
{
    if (settings->gpu_instrumentation_level >= sl->level)
    {
        auto& entry = log->leave(reference, ProfileLog::GPU);
        entry.sourceLocation = sl;
        entry.object = object;

        if (commandBuffer.gpuStats) commandBuffer.gpuStats->writeGpuTimestamp(commandBuffer, reference, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
}

void Profiler::finish() const
{
    for (auto& gpuStats : perFrameGPUStats)
    {
        getGpuResults(gpuStats);
    }
}
