/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/rtx/DrawMeshTasksIndirectCount.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Extensions.h>

#include <iostream>

using namespace vsg;

DrawMeshTasksIndirectCount::DrawMeshTasksIndirectCount()
{
}

DrawMeshTasksIndirectCount::DrawMeshTasksIndirectCount(ref_ptr<Data> in_bufferData, ref_ptr<Data> in_countBufferData, uint32_t in_maxDrawCount, uint32_t in_stride) :
    buffer(BufferInfo::create(in_bufferData)),
    countBuffer(BufferInfo::create(in_countBufferData)),
    maxDrawCount(in_maxDrawCount),
    stride(in_stride)
{
}


void DrawMeshTasksIndirectCount::read(Input& input)
{
    input.readObject("buffer.data", buffer->data);
    if (!buffer->data)
    {
        input.read("buffer.buffer", buffer->buffer);
        input.readValue<uint32_t>("buffer.offset", buffer->offset);
        input.readValue<uint32_t>("buffer.range", buffer->range);
    }

    input.read("maxDrawCount", maxDrawCount);
    input.read("stride", stride);
}

void DrawMeshTasksIndirectCount::write(Output& output) const
{
    output.writeObject("buffer.data", buffer->data);
    if (!buffer->data)
    {
        output.write("buffer,buffer", buffer->buffer);
        output.writeValue<uint32_t>("buffer.offset", buffer->offset);
        output.writeValue<uint32_t>("buffer.range", buffer->range);
    }

    output.writeObject("countBuffer.data", countBuffer->data);
    if (!countBuffer->data)
    {
        output.write("countBuffer.buffer", countBuffer->buffer);
        output.writeValue<uint32_t>("countBuffer.offset", countBuffer->offset);
        output.writeValue<uint32_t>("countBuffer.range", countBuffer->range);
    }

    output.write("maxDrawCount", maxDrawCount);
    output.write("stride", stride);
}

void DrawMeshTasksIndirectCount::compile(Context& context)
{
    if ((!buffer->buffer && buffer->data) || (!countBuffer->buffer && countBuffer->data))
    {
        auto bufferInfoList = vsg::createBufferAndTransferData(context, {buffer->data, countBuffer->data}, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
        if (bufferInfoList.size() == 2)
        {
            buffer = bufferInfoList[0];
            countBuffer = bufferInfoList[1];
        }
    }
}

void DrawMeshTasksIndirectCount::record(vsg::CommandBuffer& commandBuffer) const
{
    Device* device = commandBuffer.getDevice();
    Extensions* extensions = Extensions::Get(device, true);
    extensions->vkCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer->buffer->vk(commandBuffer.deviceID), buffer->offset, countBuffer->buffer->vk(commandBuffer.deviceID), countBuffer->offset, maxDrawCount, stride);
}
