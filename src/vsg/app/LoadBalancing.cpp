/* <editor-fold desc="MIT License">

Copyright(c) 2026 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/LoadBalancing.h>
#include <vsg/app/Viewer.h>
#include <vsg/io/Logger.h>
#include <vsg/utils/Profiler.h>

using namespace vsg;

LoadBalancing::LoadBalancing()
{
}

LoadBalancing::LoadBalancing(const LoadBalancing& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop)
{
}

LoadBalancing::~LoadBalancing()
{
}

void LoadBalancing::update(Viewer& viewer)
{
    if (previousFrameTime == std::numeric_limits<time_point>::max())
    {
        previousFrameTime = viewer.getFrameStamp()->time;
        return;
    }

    double frameDelta = (previousFrameTime == std::numeric_limits<time_point>::max()) ? 0.0 : std::chrono::duration<double, std::chrono::seconds::period>(viewer.getFrameStamp()->time - previousFrameTime).count();
    uint64_t previousFrameCount = viewer.getFrameStamp()->frameCount - 1;

    // vsg::info("LoadBalancing::update( ", &viewer, " ) ", viewer.getFrameStamp()->frameCount, ", ", frameDelta);

    double frameTargetRatio = frameDelta/targetFrameTime;
    double cpuTargetRatio = 0.0;
    double gpuTargetRatio = 0.0;

    if (auto profiler = viewer.instrumentation.cast<Profiler>())
    {
        auto& log = profiler->log;

        size_t targetFrameCount = 16;

        double max_cpu_durection = 0.0;
        double max_gpu_durection = 0.0;

        auto& frameIndices = log->frameIndices;
        auto last_frame = frameIndices.end();
        auto first_frame = (frameIndices.size() > targetFrameCount) ? (frameIndices.end() - targetFrameCount) : frameIndices.begin();
        for(auto itr = first_frame; itr != last_frame; ++itr)
        {
            auto first_index = *itr;
            const auto& first = log->entry(first_index);
            auto last_index = first.reference;

            auto cpu_duration = 0.0;
            auto gpu_duration = 0.0;

            for(auto i = first_index; i <= last_index;)
            {
                if (log->entry(i).type == ProfileLog::COMMAND_BUFFER)
                {
                    const auto& start = log->entry(i);
                    const auto& end = log->entry(start.reference);
                    if (start.gpuTime != 0 && end.gpuTime != 0)
                    {
                        gpu_duration = static_cast<double>(end.gpuTime - start.gpuTime) * log->timestampScaleToMilliseconds * 0.001;
                        if (gpu_duration > max_gpu_durection)
                        {
                            max_gpu_durection = gpu_duration;
                        }
                    }

                    cpu_duration = std::abs(std::chrono::duration<double, std::chrono::seconds::period>(end.cpuTime - start.cpuTime).count());

                    if (cpu_duration > max_cpu_durection)
                    {
                        max_cpu_durection = cpu_duration;
                    }

                    i = start.reference+1;
                }
                else
                {
                    ++i;
                }
            }



#if 0
            if (first.sourceLocation) vsg::info("    entry(", first_index,") { ", int(first.type), ", ", first.reference, ", func=", first.sourceLocation->function, ", cpu_duration = ", cpu_duration, ", gpu_duration = ",  gpu_duration, "} ");
            else vsg::info("    entry(", first_index,") { ", int(first.type), ", ", first.reference, " }");

            if (max_gpu_durection > targetGPUTime)
            {
                vsg::info("   GPU time too high ", max_gpu_durection, "ms is greated than target ", targetGPUTime, "ms");
            }
#endif
        }

        cpuTargetRatio = max_cpu_durection/targetCPUTime;
        gpuTargetRatio = max_gpu_durection/targetGPUTime;
    }

    double maxTargetRatio = std::max({frameTargetRatio, cpuTargetRatio, gpuTargetRatio});

    if (maxTargetRatio > 2.0)
    {
        vsg::info("Frame break on frameCount = ", previousFrameCount, " : maxTargetRatio = ", maxTargetRatio, " { frame = ", frameTargetRatio, ", cpu = ", cpuTargetRatio, ", gpu = ", gpuTargetRatio, " }");
    }
    else if (maxTargetRatio > 1.2)
    {
        vsg::info("Frame targets exceeded on frameCount = ", previousFrameCount,  " : maxTargetRatio = ", maxTargetRatio, " { frame = ", frameTargetRatio, ", cpu = ", cpuTargetRatio, ", gpu = ", gpuTargetRatio, " }");
    }

    previousFrameTime = viewer.getFrameStamp()->time;
}


