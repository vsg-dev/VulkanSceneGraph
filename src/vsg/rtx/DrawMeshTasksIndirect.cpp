/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/rtx/DrawMeshTasksIndirect.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Extensions.h>

#include <iostream>

using namespace vsg;

DrawMeshTasksIndirect::DrawMeshTasksIndirect()
{
}

void DrawMeshTasksIndirect::read(Input& input)
{
    input.readObject("data", bufferInfo.data);
    if (!bufferInfo.data)
    {
        input.read("buffer", bufferInfo.buffer);
        input.readValue<uint32_t>("offset", bufferInfo.offset);
        input.readValue<uint32_t>("range", bufferInfo.range);
    }

    input.read("drawCount", drawCount);
    input.read("stride", stride);
}

void DrawMeshTasksIndirect::write(Output& output) const
{
    output.writeObject("data", bufferInfo.data);
    if (!bufferInfo.data)
    {
        output.write("buffer", bufferInfo.buffer);
        output.writeValue<uint32_t>("offset", bufferInfo.offset);
        output.writeValue<uint32_t>("range", bufferInfo.range);
    }

    output.write("drawCount", drawCount);
    output.write("stride", stride);
}

void DrawMeshTasksIndirect::compile(Context& context)
{
    if (!bufferInfo.buffer && bufferInfo.data)
    {
        auto bufferInfoList = vsg::createBufferAndTransferData(context, {bufferInfo.data}, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
        if (!bufferInfoList.empty())
        {
            bufferInfo = bufferInfoList.back();
        }
    }
}

void DrawMeshTasksIndirect::record(vsg::CommandBuffer& commandBuffer) const
{
    Device* device = commandBuffer.getDevice();
    Extensions* extensions = Extensions::Get(device, true);
    extensions->vkCmdDrawMeshTasksIndirectNV(commandBuffer, bufferInfo.buffer->vk(commandBuffer.deviceID), bufferInfo.offset, drawCount, stride);
}
