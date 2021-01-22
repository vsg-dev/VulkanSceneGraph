#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/Command.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{
    /// Equivalent to VkDrawIndirectCommand that adds read/write support
    struct DrawIndirectCommand
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

        void read(vsg::Input& input)
        {
            input.read("vertexCount", vertexCount);
            input.read("instanceCount", instanceCount);
            input.read("firstVertex", firstVertex);
            input.read("firstInstance", firstInstance);
        }

        void write(vsg::Output& output) const
        {
            output.write("vertexCount", vertexCount);
            output.write("instanceCount", instanceCount);
            output.write("firstVertex", firstVertex);
            output.write("firstInstance", firstInstance);
        }
    };

    template<>
    constexpr bool has_read_write<DrawIndirectCommand>() { return true; }

    VSG_array(DrawIndirectCommandArray, DrawIndirectCommand);

} // namespace vsg
