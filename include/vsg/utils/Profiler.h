/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#pragma once

#include <vsg/io/stream.h>
#include <vsg/ui/FrameStamp.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/vk/Device.h>

namespace vsg
{
    class VSG_DECLSPEC ProfileLog : public Inherit<Object, ProfileLog>
    {
    public:
        explicit ProfileLog(size_t size = 16384);

        enum Type : uint8_t
        {
            NO_TYPE,
            FRAME,
            CPU,
            COMMAND_BUFFER,
            GPU
        };

        struct Entry
        {
            Type type = {};
            bool enter = true;
            vsg::time_point cpuTime = {};
            uint64_t gpuTime = 0;
            const SourceLocation* sourceLocation = nullptr;
            const Object* object = nullptr;
            uint64_t reference = 0;
            std::thread::id thread_id = {};
        };

        std::map<std::thread::id, std::string> threadNames;
        std::vector<Entry> entries;
        std::atomic_uint64_t index = 0;
        std::vector<uint64_t> frameIndices;
        double timestampScaleToMilliseconds = 1e-6;

        Entry& enter(uint64_t& reference, Type type)
        {
            reference = index.fetch_add(1);
            Entry& enter_entry = entry(reference);
            enter_entry.enter = true;
            enter_entry.type = type;
            enter_entry.reference = 0;
            enter_entry.cpuTime = clock::now();
            enter_entry.gpuTime = 0;
            enter_entry.thread_id = std::this_thread::get_id();
            return enter_entry;
        }

        Entry& leave(uint64_t& reference, Type type)
        {
            Entry& enter_entry = entry(reference);

            uint64_t new_reference = index.fetch_add(1);
            Entry& leave_entry = entry(new_reference);

            enter_entry.reference = new_reference;
            leave_entry.cpuTime = clock::now();
            leave_entry.gpuTime = 0;
            leave_entry.enter = false;
            leave_entry.type = type;
            leave_entry.reference = reference;
            leave_entry.thread_id = std::this_thread::get_id();
            reference = new_reference;
            return leave_entry;
        }

        Entry& entry(uint64_t reference)
        {
            return entries[reference % entries.size()];
        }

        void report(std::ostream& out);
        uint64_t report(std::ostream& out, uint64_t reference);

    public:
        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(ProfileLog)

    /// resources for collecting GPU stats for a single device on a single frame
    class VSG_DECLSPEC GPUStatsCollection : public Inherit<Object, GPUStatsCollection>
    {
    public:
        ref_ptr<Device> device;
        mutable ref_ptr<QueryPool> queryPool;
        mutable std::atomic<uint32_t> queryIndex = 0;
        std::vector<uint64_t> references;
        std::vector<uint64_t> timestamps;

        void writeGpuTimestamp(CommandBuffer& commandBuffer, uint64_t reference, VkPipelineStageFlagBits pipelineStage);
    };
    VSG_type_name(GPUStatsCollection)

    class VSG_DECLSPEC Profiler : public Inherit<Instrumentation, Profiler>
    {
    public:
        struct Settings : public Inherit<Object, Settings>
        {
            unsigned int cpu_instrumentation_level = 1;
            unsigned int gpu_instrumentation_level = 1;
            uint32_t log_size = 16384;
            uint32_t gpu_timestamp_size = 1024;
        };

        explicit Profiler(ref_ptr<Settings> in_settings = {});

        ref_ptr<Settings> settings;
        mutable ref_ptr<ProfileLog> log;


        /// resources for collecting GPU stats for all devices for a single frame
        struct FrameStatsCollection
        {
            FrameStatsCollection() {}

            std::vector<ref_ptr<GPUStatsCollection>> gpuStats;
        };

        mutable size_t frameIndex = 0;
        mutable std::vector<FrameStatsCollection> perFrameGPUStats;

        VkResult getGpuResults(FrameStatsCollection& frameStats) const;

    public:
        void setThreadName(const std::string& /*name*/) const override;

        void enterFrame(const SourceLocation* /*sl*/, uint64_t& /*reference*/, FrameStamp& /*frameStamp*/) const override;
        void leaveFrame(const SourceLocation* /*sl*/, uint64_t& /*reference*/, FrameStamp& /*frameStamp*/) const override;

        void enter(const SourceLocation* /*sl*/, uint64_t& /*reference*/, const Object* /*object*/ = nullptr) const override;
        void leave(const SourceLocation* /*sl*/, uint64_t& /*reference*/, const Object* /*object*/ = nullptr) const override;

        void enterCommandBuffer(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const override;
        void leaveCommandBuffer(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const override;

        void enter(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/, const Object* /*object*/ = nullptr) const override;
        void leave(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/, const Object* /*object*/ = nullptr) const override;

        void finish() const override;
    };
    VSG_type_name(Profiler)
} // namespace vsg
