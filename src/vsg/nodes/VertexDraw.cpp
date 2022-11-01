/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/vk/Context.h>

using namespace vsg;

VertexDraw::VertexDraw()
{
}

VertexDraw::~VertexDraw()
{
}

void VertexDraw::assignArrays(const DataList& arrayData)
{
    arrays.clear();
    arrays.reserve(arrayData.size());
    for (auto& data : arrayData)
    {
        arrays.push_back(BufferInfo::create(data));
    }
}

void VertexDraw::read(Input& input)
{
    _vulkanData.clear();

    Command::read(input);

    input.read("firstBinding", firstBinding);

    DataList dataList;
    dataList.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : dataList)
    {
        input.readObject("Array", array);
    }
    assignArrays(dataList);

    // vkCmdDraw settings
    input.read("vertexCount", vertexCount);
    input.read("instanceCount", instanceCount);
    input.read("firstVertex", firstVertex);
    input.read("firstInstance", firstInstance);
}

void VertexDraw::write(Output& output) const
{
    Command::write(output);

    output.write("firstBinding", firstBinding);
    output.writeValue<uint32_t>("NumArrays", arrays.size());
    for (auto& array : arrays)
    {
        if (array)
            output.writeObject("Array", array->data.get());
        else
            output.writeObject("Array", nullptr);
    }

    // vkCmdDraw settings
    output.write("vertexCount", vertexCount);
    output.write("instanceCount", instanceCount);
    output.write("firstVertex", firstVertex);
    output.write("firstInstance", firstInstance);
}

void VertexDraw::compile(Context& context)
{
    if (arrays.empty())
    {
        // VertexDraw does not contain required arrays
        return;
    }

    auto deviceID = context.deviceID;

    bool requiresCreateAndCopy = false;
    for (auto& array : arrays)
    {
        if (array->requiresCopy(deviceID))
        {
            requiresCreateAndCopy = true;
            break;
        }
    }

    if (requiresCreateAndCopy)
    {
        BufferInfoList combinedBufferInfos(arrays);
        createBufferAndTransferData(context, combinedBufferInfos, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);

        // info("VertexDraw::compile() create and copy ", this);
    }
    else
    {
        // info("VertexDraw::compile() no need to create and copy ", this);
    }

    assignVulkanArrayData(deviceID, arrays, _vulkanData[deviceID]);
}

void VertexDraw::record(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];

    VkCommandBuffer cmdBuffer{commandBuffer};

    vkCmdBindVertexBuffers(cmdBuffer, firstBinding, static_cast<uint32_t>(vkd.vkBuffers.size()), vkd.vkBuffers.data(), vkd.offsets.data());
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
