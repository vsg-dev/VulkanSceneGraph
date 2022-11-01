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

    /// DrawIndexedIndirect command encapsulates vkCmdDrawIndexedIndirect call and associated settings.
    class VSG_DECLSPEC DrawIndexedIndirect : public Inherit<Command, DrawIndexedIndirect>
    {
    public:
        DrawIndexedIndirect();

        DrawIndexedIndirect(ref_ptr<Data> data, uint32_t in_drawCount, uint32_t in_stride);

        DrawIndexedIndirect(ref_ptr<Buffer> in_buffer, VkDeviceSize in_offset, uint32_t in_drawCount, uint32_t in_stride);

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;
        void record(CommandBuffer& commandBuffer) const override;

        ref_ptr<BufferInfo> bufferInfo;
        uint32_t drawCount = 0;
        uint32_t stride = 0;
    };
    VSG_type_name(vsg::DrawIndexedIndirect);

} // namespace vsg
