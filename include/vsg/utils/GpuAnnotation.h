#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/utils/Instrumentation.h>

namespace vsg
{

    /// GpuAnnotationo is a vsg::Instrumentation subclasses that uses VK_debug_utils to emit annotation of the scene graph traversal.
    /// Provides tools like RenderDoc a way to report the source location associated with Vulkan calls.
    class VSG_DECLSPEC GpuAnnotation : public Inherit<Instrumentation, GpuAnnotation>
    {
    public:
        GpuAnnotation();

        enum LabelType
        {
            SourceLocation_name,
            SourceLocation_function,
            Object_className,
        };

        LabelType labelType = SourceLocation_name;

        void enterCommandBuffer(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const override;
        void leaveCommandBuffer(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const override;

        void enter(const vsg::SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer, const Object* object) const override;
        void leave(const vsg::SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer, const Object* object) const override;

    protected:
        virtual ~GpuAnnotation();
    };
    VSG_type_name(vsg::GpuAnnotation);

} // namespace vsg
