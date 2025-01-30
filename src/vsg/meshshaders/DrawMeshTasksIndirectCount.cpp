/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/meshshaders/DrawMeshTasksIndirectCount.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

DrawMeshTasksIndirectCount::DrawMeshTasksIndirectCount() :
    drawParameters(BufferInfo::create()),
    drawCount(BufferInfo::create())
{
}

DrawMeshTasksIndirectCount::DrawMeshTasksIndirectCount(ref_ptr<Data> in_drawParametersData, ref_ptr<Data> in_drawCountData, uint32_t in_maxDrawCount, uint32_t in_stride) :
    drawParameters(BufferInfo::create(in_drawParametersData)),
    drawCount(BufferInfo::create(in_drawCountData)),
    maxDrawCount(in_maxDrawCount),
    stride(in_stride)
{
}

void DrawMeshTasksIndirectCount::read(Input& input)
{
    input.readObject("drawParameters.data", drawParameters->data);
    if (!drawParameters->data)
    {
        input.read("drawParameters.buffer", drawParameters->buffer);
        input.readValue<uint32_t>("drawParameters.offset", drawParameters->offset);
        input.readValue<uint32_t>("drawParameters.range", drawParameters->range);
    }

    input.readObject("drawCount.data", drawCount->data);
    if (!drawCount->data)
    {
        input.read("drawCount.buffer", drawCount->buffer);
        input.readValue<uint32_t>("drawCount.offset", drawCount->offset);
        input.readValue<uint32_t>("drawCount.range", drawCount->range);
    }

    input.read("maxDrawCount", maxDrawCount);
    input.read("stride", stride);
}

void DrawMeshTasksIndirectCount::write(Output& output) const
{
    output.writeObject("drawParameters.data", drawParameters->data);
    if (!drawParameters->data)
    {
        output.write("drawParameters.buffer", drawParameters->buffer);
        output.writeValue<uint32_t>("drawParameters.offset", drawParameters->offset);
        output.writeValue<uint32_t>("drawParameters.range", drawParameters->range);
    }

    output.writeObject("drawCount.data", drawCount->data);
    if (!drawCount->data)
    {
        output.write("drawCount.buffer", drawCount->buffer);
        output.writeValue<uint32_t>("drawCount.offset", drawCount->offset);
        output.writeValue<uint32_t>("drawCount.range", drawCount->range);
    }

    output.write("maxDrawCount", maxDrawCount);
    output.write("stride", stride);
}

void DrawMeshTasksIndirectCount::compile(Context& context)
{
    if ((!drawParameters->buffer && drawParameters->data) || (!drawCount->buffer && drawCount->data))
    {
        createBufferAndTransferData(context, {drawParameters, drawCount}, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    }
}

void DrawMeshTasksIndirectCount::record(vsg::CommandBuffer& commandBuffer) const
{
    Device* device = commandBuffer.getDevice();
    auto extensions = device->getExtensions();
    extensions->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawParameters->buffer->vk(commandBuffer.deviceID), drawParameters->offset, drawCount->buffer->vk(commandBuffer.deviceID), drawCount->offset, maxDrawCount, stride);
}
