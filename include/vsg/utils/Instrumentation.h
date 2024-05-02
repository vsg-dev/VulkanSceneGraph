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

    // forward declare
    class CommandBuffer;
    class FrameStamp;

    /// uint_color struct used to provide a {r, g, b, a} interface a colors assigned as uint32_t
    struct uint_color
    {
        // BGRA order required to map to Tracy's uint32_t color value
        uint8_t b = 255, g = 255, r = 255, a = 255;
        constexpr uint_color() = default;
        constexpr uint_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
            b(blue), g(green), r(red), a(alpha) {}
    };

    /// SourceLocation structs mark the location in a source file when instrumentation is placed.
    /// Memory layout was chosen to be compatible to Tracy's SourceLocationData object.
    struct SourceLocation
    {
        const char* name;
        const char* function;
        const char* file;
        uint32_t line;
        uint_color color;
        uint32_t level;
    };

    /// base class for Instrumentation implentations
    class VSG_DECLSPEC Instrumentation : public Inherit<Object, Instrumentation>
    {
    public:
        Instrumentation();

        virtual ref_ptr<Instrumentation> shareOrDuplicateForThreadSafety() { return ref_ptr<Instrumentation>(this); }

        virtual void setThreadName(const std::string& /*name*/) const {};

        virtual void enterFrame(const SourceLocation* /*sl*/, uint64_t& /*reference*/, FrameStamp& /*frameStamp*/) const {};
        virtual void leaveFrame(const SourceLocation* /*sl*/, uint64_t& /*reference*/, FrameStamp& /*frameStamp*/) const {};

        virtual void enter(const SourceLocation* /*sl*/, uint64_t& /*reference*/, const Object* /*object*/ = nullptr) const {};
        virtual void leave(const SourceLocation* /*sl*/, uint64_t& /*reference*/, const Object* /*object*/ = nullptr) const {};

        virtual void enterCommandBuffer(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const {};
        virtual void leaveCommandBuffer(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/) const {};

        virtual void enter(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/, const Object* /*object*/ = nullptr) const {};
        virtual void leave(const SourceLocation* /*sl*/, uint64_t& /*reference*/, CommandBuffer& /*commandBuffer*/, const Object* /*object*/ = nullptr) const {};

        virtual void finish() const {};

    protected:
        virtual ~Instrumentation();
    };
    VSG_type_name(vsg::Instrumentation);

    /// convinience static method for sharing or duplicating Instrumentation if a valid Instrumentation object is provided
    extern ref_ptr<Instrumentation> shareOrDuplicateForThreadSafety(ref_ptr<Instrumentation> instrumentation);

    struct CpuInstrumentation
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        const Object* object;

        inline CpuInstrumentation(const Instrumentation* in_instr, const SourceLocation* in_sl, const Object* in_object) :
            instr(in_instr), sl(in_sl), reference(0), object(in_object)
        {
            if (instr) instr->enter(sl, reference, object);
        }
        inline ~CpuInstrumentation()
        {
            if (instr) instr->leave(sl, reference, object);
        }
    };

    struct GpuInstrumentation
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        CommandBuffer& commandBuffer;
        const Object* object;

        inline GpuInstrumentation(const Instrumentation* in_instr, const SourceLocation* in_sl, CommandBuffer& in_commandBuffer, const Object* in_object) :
            instr(in_instr), sl(in_sl), reference(0), commandBuffer(in_commandBuffer), object(in_object)
        {
            if (instr) instr->enter(sl, reference, commandBuffer, object);
        }
        inline ~GpuInstrumentation()
        {
            if (instr) instr->leave(sl, reference, commandBuffer, object);
        }
    };

    struct CommandBufferInstrumentation
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        CommandBuffer& commandBuffer;

        inline CommandBufferInstrumentation(const Instrumentation* in_instr, const SourceLocation* in_sl, CommandBuffer& in_commandBuffer) :
            instr(in_instr), sl(in_sl), reference(0), commandBuffer(in_commandBuffer)
        {
            if (instr) instr->enterCommandBuffer(sl, reference, commandBuffer);
        }
        inline ~CommandBufferInstrumentation()
        {
            if (instr) instr->leaveCommandBuffer(sl, reference, commandBuffer);
        }
    };

// standard colours specified in {r, g, b, a} ordering
#define COLOR_VIEWER uint_color(127, 240, 240, 255)
#define COLOR_UPDATE uint_color(0, 255, 0, 255)
#define COLOR_GPU uint_color(255, 127, 0, 255)
#define COLOR_RECORD_L1 uint_color(140, 247, 0, 255)
#define COLOR_RECORD_L2 uint_color(176, 176, 0, 255)
#define COLOR_RECORD_L3 COLOR_GPU
#define COLOR_RECORD COLOR_RECORD_L1
#define COLOR_COMPILE uint_color(255, 249, 64, 255)
#define COLOR_PAGER uint_color(240, 255, 64, 255)
#define COLOR_READ uint_color(0, 255, 128, 255)
#define COLOR_WRITE uint_color(0, 128, 255, 255)

#if defined(__clang__) || defined(__GNUC__)
#    define VsgFunctionName __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#    define VsgFunctionName __FUNCSIG__
#endif

#define __CPU_INSTRUMENTATION(level, instrumentation, name, color, object)                                                     \
    static constexpr SourceLocation s_cpu_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color, level}; \
    CpuInstrumentation __cpu_scoped_instrumentation_##__LINE__(instrumentation, &(s_cpu_source_location_##__LINE__), object);

#define __GPU_INSTRUMENTATION(level, instrumentation, cg, name, color, object)                                                 \
    static constexpr SourceLocation s_gpu_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color, level}; \
    GpuInstrumentation __gpu_scoped_instrumentation_##__LINE__(instrumentation, &(s_gpu_source_location_##__LINE__), cg, object);

#define __COMMAND_BUFFER_INSTRUMENTATION(level, instrumentation, cg, name, color)                                             \
    static constexpr SourceLocation s_cg_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color, level}; \
    CommandBufferInstrumentation __cg_scoped_instrumentation_##__LINE__(instrumentation, &(s_cg_source_location_##__LINE__), cg);

#if VSG_MAX_INSTRUMENTATION_LEVEL >= 1

#    define CPU_INSTRUMENTATION_L1(instrumentation) __CPU_INSTRUMENTATION(1, instrumentation, nullptr, uint_color(255, 255, 255, 255), nullptr)
#    define CPU_INSTRUMENTATION_L1_N(instrumentation, name) __CPU_INSTRUMENTATION(1, instrumentation, name, uint_color(255, 255, 255, 255), nullptr)
#    define CPU_INSTRUMENTATION_L1_C(instrumentation, color) __CPU_INSTRUMENTATION(1, instrumentation, nullptr, color, nullptr)
#    define CPU_INSTRUMENTATION_L1_NC(instrumentation, name, color) __CPU_INSTRUMENTATION(1, instrumentation, name, color, nullptr)

#    define GPU_INSTRUMENTATION_L1(instrumentation, cg) __GPU_INSTRUMENTATION(1, instrumentation, cg, nullptr, uint_color(255, 255, 255, 255), nullptr)
#    define GPU_INSTRUMENTATION_L1_N(instrumentation, cg, name) __GPU_INSTRUMENTATION(1, instrumentation, cg, name, uint_color(255, 255, 255, 255), nullptr)
#    define GPU_INSTRUMENTATION_L1_C(instrumentation, cg, color) __GPU_INSTRUMENTATION(1, instrumentation, cg, nullptr, color, nullptr)
#    define GPU_INSTRUMENTATION_L1_NC(instrumentation, cg, name, color) __GPU_INSTRUMENTATION(1, instrumentation, cg, name, color, nullptr)

#    define CPU_INSTRUMENTATION_L1_O(instrumentation, object) __CPU_INSTRUMENTATION(1, instrumentation, nullptr, uint_color(255, 255, 255, 255), object)
#    define CPU_INSTRUMENTATION_L1_NO(instrumentation, name, object) __CPU_INSTRUMENTATION(1, instrumentation, name, uint_color(255, 255, 255, 255), object)
#    define CPU_INSTRUMENTATION_L1_CO(instrumentation, color, object) __CPU_INSTRUMENTATION(1, instrumentation, nullptr, color, object)
#    define CPU_INSTRUMENTATION_L1_NCO(instrumentation, name, color, object) __CPU_INSTRUMENTATION(1, instrumentation, name, color, object)

#    define GPU_INSTRUMENTATION_L1_O(instrumentation, cg, object) __GPU_INSTRUMENTATION(1, instrumentation, cg, nullptr, uint_color(255, 255, 255, 255), object)
#    define GPU_INSTRUMENTATION_L1_NO(instrumentation, cg, name, object) __GPU_INSTRUMENTATION(1, instrumentation, cg, name, uint_color(255, 255, 255, 255), object)
#    define GPU_INSTRUMENTATION_L1_CO(instrumentation, cg, color, object) __GPU_INSTRUMENTATION(1, instrumentation, cg, nullptr, color, object)
#    define GPU_INSTRUMENTATION_L1_NCO(instrumentation, cg, name, color, object) __GPU_INSTRUMENTATION(1, instrumentation, cg, name, color, object)

#    define COMMAND_BUFFER_INSTRUMENTATION(instrumentation, cg, name, color) __COMMAND_BUFFER_INSTRUMENTATION(1, instrumentation, cg, name, color)

#else

#    define CPU_INSTRUMENTATION_L1(instrumentation)
#    define CPU_INSTRUMENTATION_L1_N(instrumentation, name)
#    define CPU_INSTRUMENTATION_L1_C(instrumentation, color)
#    define CPU_INSTRUMENTATION_L1_NC(instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L1(instrumentation, cg)
#    define GPU_INSTRUMENTATION_L1_N(instrumentation, cg, name)
#    define GPU_INSTRUMENTATION_L1_C(instrumentation, cg, color)
#    define GPU_INSTRUMENTATION_L1_NC(instrumentation, cg, name, color)

#    define CPU_INSTRUMENTATION_L1_O(instrumentation, object)
#    define CPU_INSTRUMENTATION_L1_NO(instrumentation, name, object)
#    define CPU_INSTRUMENTATION_L1_CO(instrumentation, color, object)
#    define CPU_INSTRUMENTATION_L1_NCO(instrumentation, name, color, object)

#    define GPU_INSTRUMENTATION_L1_O(instrumentation, cg, object)
#    define GPU_INSTRUMENTATION_L1_NO(instrumentation, cg, name, object)
#    define GPU_INSTRUMENTATION_L1_CO(instrumentation, cg, color, object)
#    define GPU_INSTRUMENTATION_L1_NCO(instrumentation, cg, name, color, object)

#    define COMMAND_BUFFER_INSTRUMENTATION(instrumentation, cg, name, color)

#endif

#if VSG_MAX_INSTRUMENTATION_LEVEL >= 2

#    define CPU_INSTRUMENTATION_L2(instrumentation) __CPU_INSTRUMENTATION(2, instrumentation, nullptr, uint_color(255, 255, 255, 255), nullptr)
#    define CPU_INSTRUMENTATION_L2_N(instrumentation, name) __CPU_INSTRUMENTATION(2, instrumentation, name, uint_color(255, 255, 255, 255), nullptr)
#    define CPU_INSTRUMENTATION_L2_C(instrumentation, color) __CPU_INSTRUMENTATION(2, instrumentation, nullptr, color, nullptr)
#    define CPU_INSTRUMENTATION_L2_NC(instrumentation, name, color) __CPU_INSTRUMENTATION(2, instrumentation, name, color, nullptr)

#    define GPU_INSTRUMENTATION_L2(instrumentation, cg) __GPU_INSTRUMENTATION(2, instrumentation, cg, nullptr, uint_color(255, 255, 255, 255), nullptr)
#    define GPU_INSTRUMENTATION_L2_N(instrumentation, cg, name) __GPU_INSTRUMENTATION(2, instrumentation, cg, name, uint_color(255, 255, 255, 255), nullptr)
#    define GPU_INSTRUMENTATION_L2_C(instrumentation, cg, color) __GPU_INSTRUMENTATION(2, instrumentation, cg, nullptr, color, nullptr)
#    define GPU_INSTRUMENTATION_L2_NC(instrumentation, cg, name, color) __GPU_INSTRUMENTATION(2, instrumentation, cg, name, color, nullptr)

#    define CPU_INSTRUMENTATION_L2_O(instrumentation, object) __CPU_INSTRUMENTATION(2, instrumentation, nullptr, uint_color(255, 255, 255, 255), object)
#    define CPU_INSTRUMENTATION_L2_NO(instrumentation, name, object) __CPU_INSTRUMENTATION(2, instrumentation, name, uint_color(255, 255, 255, 255), object)
#    define CPU_INSTRUMENTATION_L2_CO(instrumentation, color, object) __CPU_INSTRUMENTATION(2, instrumentation, nullptr, color, object)
#    define CPU_INSTRUMENTATION_L2_NCO(instrumentation, name, color, object) __CPU_INSTRUMENTATION(2, instrumentation, name, color, object)

#    define GPU_INSTRUMENTATION_L2_O(instrumentation, cg, object) __GPU_INSTRUMENTATION(2, instrumentation, cg, nullptr, uint_color(255, 255, 255, 255), object)
#    define GPU_INSTRUMENTATION_L2_NO(instrumentation, cg, name, object) __GPU_INSTRUMENTATION(2, instrumentation, cg, name, uint_color(255, 255, 255, 255), object)
#    define GPU_INSTRUMENTATION_L2_CO(instrumentation, cg, color, object) __GPU_INSTRUMENTATION(2, instrumentation, cg, nullptr, color, object)
#    define GPU_INSTRUMENTATION_L2_NCO(instrumentation, cg, name, color, object) __GPU_INSTRUMENTATION(2, instrumentation, cg, name, color, object)

#else

#    define CPU_INSTRUMENTATION_L2(instrumentation)
#    define CPU_INSTRUMENTATION_L2_N(instrumentation, name)
#    define CPU_INSTRUMENTATION_L2_C(instrumentation, color)
#    define CPU_INSTRUMENTATION_L2_NC(instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L2(instrumentation, cg)
#    define GPU_INSTRUMENTATION_L2_N(instrumentation, cg, name)
#    define GPU_INSTRUMENTATION_L2_C(instrumentation, cg, color)
#    define GPU_INSTRUMENTATION_L2_NC(instrumentation, cg, name, color)

#    define CPU_INSTRUMENTATION_L2_O(instrumentation, object)
#    define CPU_INSTRUMENTATION_L2_NO(instrumentation, name, object)
#    define CPU_INSTRUMENTATION_L2_CO(instrumentation, color, object)
#    define CPU_INSTRUMENTATION_L2_NCO(instrumentation, name, color, object)

#    define GPU_INSTRUMENTATION_L2_O(instrumentation, cg, object)
#    define GPU_INSTRUMENTATION_L2_NO(instrumentation, cg, name, object)
#    define GPU_INSTRUMENTATION_L2_CO(instrumentation, cg, color, object)
#    define GPU_INSTRUMENTATION_L2_NCO(instrumentation, cg, name, color, object)

#endif

#if VSG_MAX_INSTRUMENTATION_LEVEL >= 3

#    define CPU_INSTRUMENTATION_L3(instrumentation) __CPU_INSTRUMENTATION(3, instrumentation, nullptr, uint_color(255, 255, 255, 255), nullptr)
#    define CPU_INSTRUMENTATION_L3_N(instrumentation, name) __CPU_INSTRUMENTATION(3, instrumentation, name, uint_color(255, 255, 255, 255), nullptr)
#    define CPU_INSTRUMENTATION_L3_C(instrumentation, color) __CPU_INSTRUMENTATION(3, instrumentation, nullptr, color, nullptr)
#    define CPU_INSTRUMENTATION_L3_NC(instrumentation, name, color) __CPU_INSTRUMENTATION(3, instrumentation, name, color, nullptr)

#    define GPU_INSTRUMENTATION_L3(instrumentation, cg) __GPU_INSTRUMENTATION(3, instrumentation, cg, nullptr, uint_color(255, 255, 255, 255), nullptr)
#    define GPU_INSTRUMENTATION_L3_N(instrumentation, cg, name) __GPU_INSTRUMENTATION(3, instrumentation, cg, name, uint_color(255, 255, 255, 255), nullptr)
#    define GPU_INSTRUMENTATION_L3_C(instrumentation, cg, color) __GPU_INSTRUMENTATION(3, instrumentation, cg, nullptr, color, nullptr)
#    define GPU_INSTRUMENTATION_L3_NC(instrumentation, cg, name, color) __GPU_INSTRUMENTATION(3, instrumentation, cg, name, color, nullptr)

#    define CPU_INSTRUMENTATION_L3_O(instrumentation, object) __CPU_INSTRUMENTATION(3, instrumentation, nullptr, uint_color(255, 255, 255, 255), object)
#    define CPU_INSTRUMENTATION_L3_NO(instrumentation, name, object) __CPU_INSTRUMENTATION(3, instrumentation, name, uint_color(255, 255, 255, 255), object)
#    define CPU_INSTRUMENTATION_L3_CO(instrumentation, color, object) __CPU_INSTRUMENTATION(3, instrumentation, nullptr, color, object)
#    define CPU_INSTRUMENTATION_L3_NCO(instrumentation, name, color, object) __CPU_INSTRUMENTATION(3, instrumentation, name, color, object)

#    define GPU_INSTRUMENTATION_L3_O(instrumentation, cg, object) __GPU_INSTRUMENTATION(3, instrumentation, cg, nullptr, uint_color(255, 255, 255, 255), object)
#    define GPU_INSTRUMENTATION_L3_NO(instrumentation, cg, name, object) __GPU_INSTRUMENTATION(3, instrumentation, cg, name, uint_color(255, 255, 255, 255), object)
#    define GPU_INSTRUMENTATION_L3_CO(instrumentation, cg, color, object) __GPU_INSTRUMENTATION(3, instrumentation, cg, nullptr, color, object)
#    define GPU_INSTRUMENTATION_L3_NCO(instrumentation, cg, name, color, object) __GPU_INSTRUMENTATION(3, instrumentation, cg, name, color, object)

#else

#    define CPU_INSTRUMENTATION_L3(instrumentation)
#    define CPU_INSTRUMENTATION_L3_N(instrumentation, name)
#    define CPU_INSTRUMENTATION_L3_C(instrumentation, color)
#    define CPU_INSTRUMENTATION_L3_NC(instrumentation, name, color)

#    define GPU_INSTRUMENTATION_L3(instrumentation, cg)
#    define GPU_INSTRUMENTATION_L3_N(instrumentation, cg, name)
#    define GPU_INSTRUMENTATION_L3_C(instrumentation, cg, color)
#    define GPU_INSTRUMENTATION_L3_NC(instrumentation, cg, name, color)

#    define CPU_INSTRUMENTATION_L3_O(instrumentation, object)
#    define CPU_INSTRUMENTATION_L3_NO(instrumentation, name, object)
#    define CPU_INSTRUMENTATION_L3_CO(instrumentation, color, object)
#    define CPU_INSTRUMENTATION_L3_NCO(instrumentation, name, color, object)

#    define GPU_INSTRUMENTATION_L3_O(instrumentation, cg, object)
#    define GPU_INSTRUMENTATION_L3_NO(instrumentation, cg, name, object)
#    define GPU_INSTRUMENTATION_L3_CO(instrumentation, cg, color, object)
#    define GPU_INSTRUMENTATION_L3_NCO(instrumentation, cg, name, color, object)

#endif

} // namespace vsg
