#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/io/Logger.h>
#include <vsg/maths/vec4.h>

namespace vsg
{

#if defined(__clang__) || defined(__GNUC__)
#    define VsgFunctionName __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#    define VsgFunctionName __FUNCSIG__
#endif

    class Device;
    class Queue;
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
    };

    /// base class for Instrumentation implentations
    class VSG_DECLSPEC Instrumentation : public Inherit<Object, Instrumentation>
    {
    public:
        Instrumentation();

        virtual void enterFrame(vsg::ref_ptr<vsg::FrameStamp> frameStamp) = 0;
        virtual void leaveFrame(vsg::ref_ptr<vsg::FrameStamp> frameStamp) = 0;

        virtual void enterCommandBuffer(vsg::ref_ptr<vsg::CommandBuffer> commandBuffer) = 0;
        virtual void leaveCommandBuffer() = 0;

        virtual void enter(const SourceLocation* sl, uint64_t& reference) const = 0;
        virtual void leave(const SourceLocation* sl, uint64_t& reference) const = 0;

        virtual void enter(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const = 0;
        virtual void leave(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const = 0;

    protected:
        virtual ~Instrumentation();
    };
    VSG_type_name(vsg::Instrumentation);

    /// Concrete Implementation that uses VK_debug_utils to emit annotation to scene graph traversal.
    /// Provides tools like RenderDoc a way to report the source location associated with Vulkan calls.
    class VSG_DECLSPEC VulkanAnnotation : public Inherit<Instrumentation, VulkanAnnotation>
    {
    public:
        VulkanAnnotation();

        void enterFrame(vsg::ref_ptr<vsg::FrameStamp>) override {};
        void leaveFrame(vsg::ref_ptr<vsg::FrameStamp>) override {};

        void enterCommandBuffer(vsg::ref_ptr<vsg::CommandBuffer>) override {}
        void leaveCommandBuffer() override {}

        void enter(const SourceLocation*, uint64_t&) const override {}
        void leave(const SourceLocation*, uint64_t&) const override {}

        void enter(const vsg::SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const override;
        void leave(const vsg::SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~VulkanAnnotation();
    };
    VSG_type_name(vsg::VulkanAnnotation);

    struct ScopedInstrumentation
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        inline ScopedInstrumentation(const Instrumentation* in_instr, const SourceLocation* in_sl) :
            instr(in_instr), sl(in_sl)
        {
            if (instr) instr->enter(sl, reference);
        }
        inline ~ScopedInstrumentation()
        {
            if (instr) instr->leave(sl, reference);
        }
    };

    struct ScopedInstrumentationCG
    {
        const Instrumentation* instr;
        const SourceLocation* sl;
        uint64_t reference;
        CommandBuffer& commandBuffer;

        inline ScopedInstrumentationCG(const Instrumentation* in_instr, const SourceLocation* in_sl, CommandBuffer& in_commandBuffer) :
            instr(in_instr), sl(in_sl), commandBuffer(in_commandBuffer)
        {
            if (instr) instr->enter(sl, reference, commandBuffer);
        }
        inline ~ScopedInstrumentationCG()
        {
            if (instr) instr->leave(sl, reference, commandBuffer);
        }
    };

#define SCOPED_INSTRUMENTATION(instrumentation)                                                                                            \
    static constexpr SourceLocation s_source_location_##__LINE__{nullptr, VsgFunctionName, __FILE__, __LINE__, ubvec4(255, 255, 255, 255)}; \
    ScopedInstrumentation __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__));
#define SCOPED_INSTRUMENTATION_N(instrumentation, name)                                                                                 \
    static constexpr SourceLocation s_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, ubvec4(255, 255, 255, 255)}; \
    ScopedInstrumentation __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__));
#define SCOPED_INSTRUMENTATION_C(instrumentation, color)                                                              \
    static constexpr SourceLocation s_source_location_##__LINE__{nullptr, VsgFunctionName, __FILE__, __LINE__, color}; \
    ScopedInstrumentation __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__));
#define SCOPED_INSTRUMENTATION_NC(instrumentation, name, color)                                                    \
    static constexpr SourceLocation s_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color}; \
    ScopedInstrumentation __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__));

#define SCOPED_INSTRUMENTATION_CG(instrumentation, cg)                                                                                            \
    static constexpr SourceLocation s_source_location_##__LINE__{nullptr, VsgFunctionName, __FILE__, __LINE__, ubvec4(255, 255, 255, 255)}; \
    ScopedInstrumentationCG __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__), cg);
#define SCOPED_INSTRUMENTATION_CG_N(instrumentation, cg, name)                                                                                 \
    static constexpr SourceLocation s_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, ubvec4(255, 255, 255, 255)}; \
    ScopedInstrumentationCG __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__), cg);
#define SCOPED_INSTRUMENTATION_CG_C(instrumentation, cg, color)                                                              \
    static constexpr SourceLocation s_source_location_##__LINE__{nullptr, VsgFunctionName, __FILE__, __LINE__, color}; \
    ScopedInstrumentationCG __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__), cg);
#define SCOPED_INSTRUMENTATION_CG_NC(instrumentation, cg, name, color)                                                    \
    static constexpr SourceLocation s_source_location_##__LINE__{name, VsgFunctionName, __FILE__, __LINE__, color}; \
    ScopedInstrumentationCG __scoped_instrumentation(instrumentation, &(s_source_location_##__LINE__), cg);


} // namespace vsg
