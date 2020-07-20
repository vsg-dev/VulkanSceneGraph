/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>
#include <vsg/io/Options.h>

using namespace vsg;

// provide definition as VK_INDEX_TYPE_UINT8_EXT is not available in all headers
#define VK_INDEX_TYPE_UINT8 static_cast<VkIndexType>(1000265000)

VkIndexType vsg::computeIndexType(const Data* indices)
{
    if (indices)
    {
        switch (indices->valueSize())
        {
        case (1): return VK_INDEX_TYPE_UINT8;
        case (2): return VK_INDEX_TYPE_UINT16;
        case (4): return VK_INDEX_TYPE_UINT32;
        default: break;
        }
    }
    // nothing valid assigned
    return VK_INDEX_TYPE_MAX_ENUM;
}

BindIndexBuffer::BindIndexBuffer(Data* indices) :
    _indices(indices)
{
}

BindIndexBuffer::BindIndexBuffer(const BufferData& bufferData)
{
    if (bufferData._buffer.valid())
    {
        _indices = bufferData._data;

        auto& vkd = _vulkanData[bufferData._buffer->getDevice()->deviceID];
        vkd.bufferData = bufferData;
        vkd.indexType = computeIndexType(bufferData._data);
    }
}

BindIndexBuffer::BindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType)
{
    auto& vkd = _vulkanData[buffer->getDevice()->deviceID];
    vkd.bufferData = BufferData(buffer, offset, 0),
    vkd.indexType = indexType;
}

BindIndexBuffer::~BindIndexBuffer()
{
    for (auto& vkd : _vulkanData)
    {
        if (vkd.bufferData._buffer)
        {
            vkd.bufferData._buffer->release(vkd.bufferData._offset, 0); // TODO, we don't locally have a size allocated
        }
    }
}

void BindIndexBuffer::read(Input& input)
{
    Command::read(input);

    // clear Vulkan objects
    _vulkanData.clear();

    // read the key indices data
    input.readObject("Indices", _indices);
}

void BindIndexBuffer::write(Output& output) const
{
    Command::write(output);

    // write indices data
    output.writeObject("Indices", _indices.get());
}

void BindIndexBuffer::compile(Context& context)
{
    // nothing to compile
    if (!_indices) return;

    auto& vkd = _vulkanData[context.deviceID];

    // check if already compiled
    if (vkd.bufferData._buffer) return;

    auto bufferDataList = vsg::createBufferAndTransferData(context, {_indices}, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    if (!bufferDataList.empty())
    {
        vkd.bufferData = bufferDataList.back();
        vkd.indexType = computeIndexType(_indices);
    }
}

void BindIndexBuffer::record(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindIndexBuffer(commandBuffer, *vkd.bufferData._buffer, vkd.bufferData._offset, vkd.indexType);
}
