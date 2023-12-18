#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/core/Version.h>
#include <vsg/io/Logger.h>
#include <vsg/maths/vec4.h>

namespace vsg
{

    class CommandBuffer;
    class FrameStamp;

    /// SourceLocation structs mark the location in a source file when instrumentation is placed.
    /// Memory layout was chosen to be compatible to Tracy's SourceLocationData object.
    struct SourceLocation
    {
        const char* name;
        const char* function;
        const char* file;
        uint32_t line;
        ubvec4 color;
        uint32_t level;
    };

    /// base class for Instrumentation implentations
    class VSG_DECLSPEC Instrumentation : public Inherit<Object, Instrumentation>
    {
    public:
        Instrumentation();

        virtual void enterFrame(const SourceLocation* /*sl*/, uint64_t& /*reference*/, FrameStamp& /*frameStamp*/) const {};
        virtual void leaveFrame(const SourceLocation* /*sl*/, uint64_t& /*reference*/, FrameStamp& /*frameStamp*/) const {};

        virtual void enter(const SourceLocation* /*sl*/, uint64_t& /*reference*/) const {};
        virtual void leave(const SourceLocation* /*sl*/, uint64_t& /*reference*/) const {};

        virtual void enterCommandBuffer(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const {};
        virtual void leaveCommandBuffer(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const {};

        virtual void enter(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const {};
        virtual void leave(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const {};

    protected:
        virtual ~Instrumentation();
    };
    VSG_type_name(vsg::Instrumentation);

    struct CpuInstrumentation
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        inline CpuInstrumentation(const Instrumentation* in_instr, const SourceLocation* in_sl) :
            instr(in_instr), sl(in_sl)
        {
            if (instr) instr->enter(sl, reference);
        }
        inline ~CpuInstrumentation()
        {
            if (instr) instr->leave(sl, reference);
        }
    };

    struct GpuInstrumentation
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        CommandBuffer& commandBuffer;

        inline GpuInstrumentation(const Instrumentation* in_instr, const SourceLocation* in_sl, CommandBuffer& in_commandBuffer) :
            instr(in_instr), sl(in_sl), commandBuffer(in_commandBuffer)
        {
            if (instr) instr->enter(sl, reference, commandBuffer);
        }
        inline ~GpuInstrumentation()
        {
            if (instr) instr->leave(sl, reference, commandBuffer);
        }
    };

    struct CommandBufferInstrumentation
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        CommandBuffer& commandBuffer;

        inline CommandBufferInstrumentation(const Instrumentation* in_instr, const SourceLocation* in_sl, CommandBuffer& in_commandBuffer) :
            instr(in_instr), sl(in_sl), commandBuffer(in_commandBuffer)
        {
            if (instr) instr->enterCommandBuffer(sl, reference, commandBuffer);
        }
        inline ~CommandBufferInstrumentation()
        {
            if (instr) instr->leaveCommandBuffer(sl, reference, commandBuffer);
        }
    };

    #if defined(__clang__) || defined(__GNUC__)
#    define VsgFunctionName __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#    define VsgFunctionName __FUNCSIG__
#endif

#define __CPU_INSTRUMENTATION(level, instrumentation, name, color)                                                         \
    static constexpr SourceLocation s_cpu_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color, level}; \
    CpuInstrumentation __cpu_scoped_instrumentation_##__LINE__(instrumentation, &(s_cpu_source_location_##__LINE__));

#define __GPU_INSTRUMENTATION(level, instrumentation, cg, name, color)                                                     \
    static constexpr SourceLocation s_gpu_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color, level}; \
    GpuInstrumentation __gpu_scoped_instrumentation_##__LINE__(instrumentation, &(s_gpu_source_location_##__LINE__), cg);

#define __COMMAND_BUFFER_INSTRUMENTATION(level, instrumentation, cg, name, color)                                          \
    static constexpr SourceLocation s_cg_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color, level}; \
    CommandBufferInstrumentation __cb_scoped_instrumentation_##__LINE__(instrumentation, &(s_cg_source_location_##__LINE__), cg);

#if VSG_MAX_INSTRUMENTATION_LEVEL >= 1

#    define CPU_INSTRUMENTATION_L1(instrumentation) __CPU_INSTRUMENTATION(1, instrumentation, nullptr, ubvec4(255, 255, 255, 255))
#    define CPU_INSTRUMENTATION_L1_N(instrumentation, name) __CPU_INSTRUMENTATION(1, instrumentation, name, ubvec4(255, 255, 255, 255))
#    define CPU_INSTRUMENTATION_L1_C(instrumentation, color) __CPU_INSTRUMENTATION(1, instrumentation, nullptr, color)
#    define CPU_INSTRUMENTATION_L1_NC(instrumentation, name, color) __CPU_INSTRUMENTATION(1, instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L1(instrumentation, cg) __GPU_INSTRUMENTATION(1, instrumentation, cg, nullptr, ubvec4(255, 255, 255, 255))
#    define GPU_INSTRUMENTATION_L1_N(instrumentation, cg, name) __GPU_INSTRUMENTATION(1, instrumentation, cg, name, ubvec4(255, 255, 255, 255))
#    define GPU_INSTRUMENTATION_L1_C(instrumentation, cg, color) __GPU_INSTRUMENTATION(1, instrumentation, cg, nullptr, color)
#    define GPU_INSTRUMENTATION_L1_NC(instrumentation, cg, name, color) __CPU_INSTRUMENTATION(1, instrumentation, name, color)

#    define COMMAND_BUFFER_INSTRUMENTATION(instrumentation, cg) __COMMAND_BUFFER_INSTRUMENTATION(1, instrumentation, cg, nullptr, ubvec4(255, 255, 255, 255))

#else

#    define CPU_INSTRUMENTATION_L1(instrumentation)
#    define CPU_INSTRUMENTATION_L1_N(instrumentation, name)
#    define CPU_INSTRUMENTATION_L1_C(instrumentation, color)
#    define CPU_INSTRUMENTATION_L1_NC(instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L1(instrumentation, cg)
#    define GPU_INSTRUMENTATION_L1_N(instrumentation, cg, name)
#    define GPU_INSTRUMENTATION_L1_C(instrumentation, cg, color)
#    define GPU_INSTRUMENTATION_L1_NC(instrumentation, cg, name, color)

#    define COMMAND_BUFFER_INSTRUMENTATION(instrumentation, cg)

#endif

#if VSG_MAX_INSTRUMENTATION_LEVEL >= 2
#    define CPU_INSTRUMENTATION_L2(instrumentation) __CPU_INSTRUMENTATION(2, instrumentation, nullptr, ubvec4(255, 255, 255, 255))
#    define CPU_INSTRUMENTATION_L2_N(instrumentation, name) __CPU_INSTRUMENTATION(2, instrumentation, name, ubvec4(255, 255, 255, 255))
#    define CPU_INSTRUMENTATION_L2_C(instrumentation, color) __CPU_INSTRUMENTATION(2, instrumentation, nullptr, color)
#    define CPU_INSTRUMENTATION_L2_NC(instrumentation, name, color) __CPU_INSTRUMENTATION(2, instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L2(instrumentation, cg) __GPU_INSTRUMENTATION(2, instrumentation, cg, nullptr, ubvec4(255, 255, 255, 255))
#    define GPU_INSTRUMENTATION_L2_N(instrumentation, cg, name) __GPU_INSTRUMENTATION(2, instrumentation, cg, name, ubvec4(255, 255, 255, 255))
#    define GPU_INSTRUMENTATION_L2_C(instrumentation, cg, color) __GPU_INSTRUMENTATION(2, instrumentation, cg, nullptr, color)
#    define GPU_INSTRUMENTATION_L2_NC(instrumentation, cg, name, color) __CPU_INSTRUMENTATION(2, instrumentation, name, color)
#else
#    define CPU_INSTRUMENTATION_L2(instrumentation)
#    define CPU_INSTRUMENTATION_L2_N(instrumentation, name)
#    define CPU_INSTRUMENTATION_L2_C(instrumentation, color)
#    define CPU_INSTRUMENTATION_L2_NC(instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L2(instrumentation, cg)
#    define GPU_INSTRUMENTATION_L2_N(instrumentation, cg, name)
#    define GPU_INSTRUMENTATION_L2_C(instrumentation, cg, color)
#    define GPU_INSTRUMENTATION_L2_NC(instrumentation, cg, name, color)
#endif

#if VSG_MAX_INSTRUMENTATION_LEVEL >= 3
#    define CPU_INSTRUMENTATION_L3(instrumentation) __CPU_INSTRUMENTATION(3, instrumentation, nullptr, ubvec4(255, 255, 255, 255))
#    define CPU_INSTRUMENTATION_L3_N(instrumentation, name) __CPU_INSTRUMENTATION(3, instrumentation, name, ubvec4(255, 255, 255, 255))
#    define CPU_INSTRUMENTATION_L3_C(instrumentation, color) __CPU_INSTRUMENTATION(3, instrumentation, nullptr, color)
#    define CPU_INSTRUMENTATION_L3_NC(instrumentation, name, color) __CPU_INSTRUMENTATION(3, instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L3(instrumentation, cg) __GPU_INSTRUMENTATION(3, instrumentation, cg, nullptr, ubvec4(255, 255, 255, 255))
#    define GPU_INSTRUMENTATION_L3_N(instrumentation, cg, name) __GPU_INSTRUMENTATION(3, instrumentation, cg, name, ubvec4(255, 255, 255, 255))
#    define GPU_INSTRUMENTATION_L3_C(instrumentation, cg, color) __GPU_INSTRUMENTATION(3, instrumentation, cg, nullptr, color)
#    define GPU_INSTRUMENTATION_L3_NC(instrumentation, cg, name, color) __CPU_INSTRUMENTATION(3, instrumentation, name, color)
#else
#    define CPU_INSTRUMENTATION_L3(instrumentation)
#    define CPU_INSTRUMENTATION_L3_N(instrumentation, name)
#    define CPU_INSTRUMENTATION_L3_C(instrumentation, color)
#    define CPU_INSTRUMENTATION_L3_NC(instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L3(instrumentation, cg)
#    define GPU_INSTRUMENTATION_L3_N(instrumentation, cg, name)
#    define GPU_INSTRUMENTATION_L3_C(instrumentation, cg, color)
#    define GPU_INSTRUMENTATION_L3_NC(instrumentation, cg, name, color)
#endif

} // namespace vsg
