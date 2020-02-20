/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/Draw.h>

#include <vsg/io/ReaderWriter.h>

#include <vsg/traversals/RecordTraversal.h>

#include <vsg/nodes/VertexIndexDraw.h>

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
    for (size_t i = 0; i < _buffers.size(); ++i)
    {
        if (_buffers[i])
        {
            _buffers[i]->release(_offsets[i], 0);
        }
    }
    if (_bufferData._buffer) _bufferData._buffer->release(_bufferData._offset, _bufferData._range);
}

void VertexIndexDraw::read(Input& input)
{
    Command::read(input);

    arrays.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : arrays)
    {
        array = input.readObject<Data>("Array");
    }

    indices = input.readObject<Data>("Indices");

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

    output.writeValue<uint32_t>("NumArrays", arrays.size());
    for (auto& array : arrays)
    {
        output.writeObject("Array", array.get());
    }

    output.writeObject("Indices", indices.get());

    // vkCmdDrawIndexed settings
    output.write("indexCount", indexCount);
    output.write("instanceCount", instanceCount);
    output.write("firstIndex", firstIndex);
    output.write("vertexOffset", vertexOffset);
    output.write("firstInstance", firstInstance);
}

void VertexIndexDraw::compile(Context& context)
{
    if (arrays.size() == 0 || !indices)
    {
        // VertexIndexDraw does not contain required arrays and/or indices
        return;
    }

    // check to see if we've already been compiled
    if (_buffers.size() == arrays.size()) return;

    bool failure = false;

    _buffers.clear();
    _vkBuffers.clear();
    _offsets.clear();

    DataList dataList;
    dataList.reserve(arrays.size() + 1);
    dataList.insert(dataList.end(), arrays.begin(), arrays.end());
    dataList.emplace_back(indices);

    auto bufferDataList = vsg::createBufferAndTransferData(context, dataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    if (!bufferDataList.empty())
    {
        BufferDataList vertexBufferData(bufferDataList.begin(), bufferDataList.begin() + arrays.size());

        for (auto& bufferData : vertexBufferData)
        {
            _buffers.push_back(bufferData._buffer);
            _vkBuffers.push_back(*(bufferData._buffer));
            _offsets.push_back(bufferData._offset);
        }

        _bufferData = bufferDataList.back();
        _indexType = computeIndexType(indices); // TODO need to check Index type
    }
    else
    {
        failure = true;
    }

    if (failure)
    {
        //std::cout << "Failed to create required arrays/indices buffers on GPU." << std::endl;
        _buffers.clear();
        _vkBuffers.clear();
        _offsets.clear();
        _bufferData = BufferData();
        return;
    }
}

void VertexIndexDraw::dispatch(CommandBuffer& commandBuffer) const
{
    VkCommandBuffer cmdBuffer{commandBuffer};

    vkCmdBindVertexBuffers(cmdBuffer, _firstBinding, static_cast<uint32_t>(_vkBuffers.size()), _vkBuffers.data(), _offsets.data());

    vkCmdBindIndexBuffer(cmdBuffer, *_bufferData._buffer, _bufferData._offset, _indexType);

    vkCmdDrawIndexed(cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
