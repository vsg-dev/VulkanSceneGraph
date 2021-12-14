/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/traversals/RecordTraversal.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
//  VertexIndexDraw node
//       vertex arrays
//       index arrays
//       draw + draw DrawIndexed
//
VertexIndexDraw::VertexIndexDraw(Allocator* allocator) :
    Inherit(allocator)
{
}

VertexIndexDraw::~VertexIndexDraw()
{
}

void VertexIndexDraw::assignArrays(const DataList& arrayData)
{
    arrays.clear();
    arrays.reserve(arrayData.size());
    for (auto& data : arrayData)
    {
        arrays.push_back(BufferInfo::create(data));
    }
}

void VertexIndexDraw::assignIndices(ref_ptr<vsg::Data> indexData)
{
    if (indexData)
        indices = BufferInfo::create(indexData);
    else
        indices = {};
}

void VertexIndexDraw::read(Input& input)
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

    ref_ptr<vsg::Data> indices_data;
    input.readObject("Indices", indices_data);

    assignIndices(indices_data);

    // vkCmdDrawIndexed settings
    input.read("indexCount", indexCount);
    input.read("instanceCount", instanceCount);
    input.read("firstIndex", firstIndex);
    input.read("vertexOffset", vertexOffset);
    input.read("firstInstance", firstInstance);
}

void VertexIndexDraw::write(Output& output) const
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

    if (indices)
        output.writeObject("Indices", indices->data.get());
    else
        output.writeObject("Indices", nullptr);

    // vkCmdDrawIndexed settings
    output.write("indexCount", indexCount);
    output.write("instanceCount", instanceCount);
    output.write("firstIndex", firstIndex);
    output.write("vertexOffset", vertexOffset);
    output.write("firstInstance", firstInstance);
}

void VertexIndexDraw::compile(Context& context)
{
    if (arrays.empty() || !indices)
    {
        // VertexIndexDraw does not contain required arrays and/or indices
        return;
    }

    auto deviceID = context.deviceID;

    bool requiresCreateAndCopy = false;
    if (indices && indices->requiresCopy(deviceID))
        requiresCreateAndCopy = true;
    else
    {
        for (auto& array : arrays)
        {
            if (array->requiresCopy(deviceID))
            {
                requiresCreateAndCopy = true;
                break;
            }
        }
    }

    if (!requiresCreateAndCopy)
    {
        return;
    }

    auto& vkd = _vulkanData[deviceID];

    vkd = {};

    BufferInfoList combinedBufferInfos(arrays);
    combinedBufferInfos.push_back(indices);

    if (createBufferAndTransferData(context, combinedBufferInfos, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE))
    {
        for (auto& bufferInfo : arrays)
        {
            vkd.vkBuffers.push_back(bufferInfo->buffer->vk(deviceID));
            vkd.offsets.push_back(bufferInfo->offset);
        }
    }

    indexType = computeIndexType(indices->data);
}

void VertexIndexDraw::record(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];

    VkCommandBuffer cmdBuffer{commandBuffer};

    vkCmdBindVertexBuffers(cmdBuffer, firstBinding, static_cast<uint32_t>(vkd.vkBuffers.size()), vkd.vkBuffers.data(), vkd.offsets.data());

    vkCmdBindIndexBuffer(cmdBuffer, indices->buffer->vk(commandBuffer.deviceID), indices->offset, indexType);

    vkCmdDrawIndexed(cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
