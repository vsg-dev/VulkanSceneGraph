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

    #if defined ( __clang__ ) || defined ( __GNUC__ )
    # define VsgFunctionName __PRETTY_FUNCTION__
    #elif defined ( _MSC_VER )
    # define VsgFunctionName __FUNCSIG__
    #endif

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

    class VSG_DECLSPEC Instrumentation : public Inherit<Object, Instrumentation>
    {
    public:

        Instrumentation();

        virtual void enter(const SourceLocation* sl) const
        {
            info("enter ", sl, " : ", sl->file, ", ", sl->line, ", ", sl->function);
        }

        virtual void leave(const SourceLocation* sl) const
        {
            info("leave ", sl);
        }

    protected:

        virtual ~Instrumentation();

    };
    VSG_type_name(vsg::Instrumentation);

    struct ScopedInstrumentation
    {
        const SourceLocation* sl;
        const Instrumentation* instr;
        inline ScopedInstrumentation(const SourceLocation* in_sl, const Instrumentation* in_instr) : sl(in_sl), instr(in_instr) { if (instr) instr->enter(sl); }
        inline ~ScopedInstrumentation() { if (instr) instr->leave(sl); }
    };

    #define SCOPED_INSTRUMENTASTION(instrumentation) static constexpr SourceLocation s_source_location_##__LINE__ { nullptr, VsgFunctionName, __FILE__, __LINE__, ubvec4(255, 255, 255, 255) }; ScopedInstrumentation __scoped_instrumentation(&(s_source_location_##__LINE__), instrumentation);


} // namespace vsg
