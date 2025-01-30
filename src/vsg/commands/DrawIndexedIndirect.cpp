/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/DrawIndexedIndirect.h>

using namespace vsg;

DrawIndexedIndirect::DrawIndexedIndirect()
{
}

DrawIndexedIndirect::DrawIndexedIndirect(ref_ptr<Data> data, uint32_t in_drawCount, uint32_t in_stride) :
    bufferInfo(BufferInfo::create(data)),
    drawCount(in_drawCount),
    stride(in_stride)
{
}

DrawIndexedIndirect::DrawIndexedIndirect(ref_ptr<Buffer> in_buffer, VkDeviceSize in_offset, uint32_t in_drawCount, uint32_t in_stride) :
    bufferInfo(BufferInfo::create(in_buffer, in_offset, in_drawCount * in_stride)),
    drawCount(in_drawCount),
    stride(in_stride)
{
}

void DrawIndexedIndirect::read(Input& input)
{
    Command::read(input);

    ref_ptr<Data> data;
    input.read("data", data);
    if (data)
    {
        bufferInfo = BufferInfo::create(data);
    }
    else
    {
        ref_ptr<Buffer> buffer;
        VkDeviceSize offset = 0;
        VkDeviceSize range = 0;
        input.readObject("buffer", bufferInfo->buffer);
        input.readValue<uint32_t>("offset", offset);
        input.readValue<uint32_t>("range", range);
        if (buffer)
        {
            bufferInfo = BufferInfo::create(buffer, offset, range);
        }
    }

    input.read("drawCount", drawCount);
    input.read("stride", stride);
}

void DrawIndexedIndirect::write(Output& output) const
{
    Command::write(output);

    if (bufferInfo)
    {
        output.writeObject("data", bufferInfo->data);
        if (!bufferInfo->data)
        {
            output.writeObject("buffer", bufferInfo->buffer);
            output.writeValue<uint32_t>("offset", bufferInfo->offset);
            output.writeValue<uint32_t>("range", bufferInfo->range);
        }
    }
    else
    {
        output.writeObject("data", nullptr);
        output.writeObject("buffer", nullptr);
        output.writeValue<uint32_t>("offset", 0);
        output.writeValue<uint32_t>("range", 0);
    }

    output.write("drawCount", drawCount);
    output.write("stride", stride);
}

void DrawIndexedIndirect::compile(Context& context)
{
    if (!bufferInfo->buffer && bufferInfo->data)
    {
        createBufferAndTransferData(context, {bufferInfo}, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    }
}

void DrawIndexedIndirect::record(CommandBuffer& commandBuffer) const
{
    vkCmdDrawIndexedIndirect(commandBuffer, bufferInfo->buffer->vk(commandBuffer.deviceID), bufferInfo->offset, drawCount, stride);
}
