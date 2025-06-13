#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/Command.h>
#include <vsg/nodes/Node.h>
#include <vsg/state/BufferInfo.h>

namespace vsg
{

    /** InstanceDraw provides a lightweight way of binding vertex arrays, indices and then issuing a vkCmdDraw command.
      * Higher performance equivalent to use of individual vsg::BindVertexBuffers, vsg::BindIndexBuffer and vsg::Draw commands.*/
    class VSG_DECLSPEC InstanceDraw : public Inherit<Command, InstanceDraw>
    {
    public:
        InstanceDraw();
        InstanceDraw(const InstanceDraw& rhs, const CopyOp& copyop = {});

        // vkCmdDraw settings
        // vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        uint32_t vertexCount = 0;
        uint32_t firstVertex = 0;

        uint32_t firstBinding = 0;
        BufferInfoList arrays;

        void assignArrays(const DataList& in_arrays);

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return InstanceDraw::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;
        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~InstanceDraw();
    };
    VSG_type_name(vsg::InstanceDraw)

} // namespace vsg
