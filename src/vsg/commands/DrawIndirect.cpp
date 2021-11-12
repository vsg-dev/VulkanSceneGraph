/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/DrawIndirect.h>
#include <vsg/io/Options.h>

using namespace vsg;

DrawIndirect::DrawIndirect()
{
}

DrawIndirect::DrawIndirect(ref_ptr<Data> data, uint32_t in_drawCount, uint32_t in_stride) :
    bufferInfo(BufferInfo::create(data)),
    drawCount(in_drawCount),
    stride(in_stride)
{
}

DrawIndirect::DrawIndirect(ref_ptr<Buffer> in_buffer, VkDeviceSize in_offset, uint32_t in_drawCount, uint32_t in_stride) :
    bufferInfo(BufferInfo::create(in_buffer, in_offset, in_drawCount * in_stride)),
    drawCount(in_drawCount),
    stride(in_stride)
{
}

void DrawIndirect::read(Input& input)
{
    Command::read(input);

    input.readObject("data", bufferInfo->data);
    if (!bufferInfo->data)
    {
        input.read("buffer", bufferInfo->buffer);
        input.readValue<uint32_t>("offset", bufferInfo->offset);
        input.readValue<uint32_t>("range", bufferInfo->range);
    }

    input.read("drawCount", drawCount);
    input.read("stride", stride);
}

void DrawIndirect::write(Output& output) const
{
    Command::write(output);

    output.writeObject("data", bufferInfo->data);
    if (!bufferInfo->data)
    {
        output.write("buffer", bufferInfo->buffer);
        output.writeValue<uint32_t>("offset", bufferInfo->offset);
        output.writeValue<uint32_t>("range", bufferInfo->range);
    }

    output.write("drawCount", drawCount);
    output.write("stride", stride);
}

void DrawIndirect::compile(Context& context)
{
    if (!bufferInfo->buffer && bufferInfo->data)
    {
        createBufferAndTransferData(context, {bufferInfo}, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    }
}

void DrawIndirect::record(CommandBuffer& commandBuffer) const
{
    vkCmdDrawIndirect(commandBuffer, bufferInfo->buffer->vk(commandBuffer.deviceID), bufferInfo->offset, drawCount, stride);
}
