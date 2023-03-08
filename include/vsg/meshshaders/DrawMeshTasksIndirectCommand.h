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
    /// Equivalent to VkDrawMeshTasksIndirectCommandEXT that adds read/write support
    struct DrawMeshTasksIndirectCommand
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;

        void read(vsg::Input& input)
        {
            input.read("groupCountX", x);
            input.read("groupCountY", y);
            input.read("groupCountZ", z);
        }

        void write(vsg::Output& output) const
        {
            output.write("groupCountX", x);
            output.write("groupCountY", y);
            output.write("groupCountZ", z);
        }
    };
    VSG_type_name(vsg::DrawMeshTasksIndirectCommand);

    template<>
    constexpr bool has_read_write<DrawMeshTasksIndirectCommand>() { return true; }

    VSG_array(DrawMeshTasksIndirectCommandArray, DrawMeshTasksIndirectCommand);

} // namespace vsg
