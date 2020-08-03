#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandBuffer.h>

#include <vsg/commands/Command.h>

namespace vsg
{

    class VSG_DECLSPEC Draw : public Inherit<Command, Draw>
    {
    public:
        Draw() {}

        Draw(uint32_t in_vertexCount, uint32_t in_instanceCount, uint32_t in_firstVertex, uint32_t in_firstInstance) :
            vertexCount(in_vertexCount),
            instanceCount(in_instanceCount),
            firstVertex(in_firstVertex),
            firstInstance(in_firstInstance) {}

        void read(Input& input) override;
        void write(Output& output) const override;

        void record(CommandBuffer& commandBuffer) const override
        {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        uint32_t vertexCount = 0;
        uint32_t instanceCount = 0;
        uint32_t firstVertex = 0;
        uint32_t firstInstance = 0;
    };
    VSG_type_name(vsg::Draw);

} // namespace vsg
