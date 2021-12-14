/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/io/Options.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

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

BindIndexBuffer::BindIndexBuffer(ref_ptr<Data> in_indices)
{
    assignIndices(in_indices);
}

BindIndexBuffer::~BindIndexBuffer()
{
}

void BindIndexBuffer::assignIndices(ref_ptr<vsg::Data> indexData)
{
    if (indexData)
        indices = BufferInfo::create(indexData);
    else
        indices = {};
}

void BindIndexBuffer::read(Input& input)
{
    Command::read(input);

    // read the key indices data
    ref_ptr<vsg::Data> indices_data;
    if (input.version_greater_equal(0, 1, 4))
    {
        input.readObject("indices", indices_data);
    }
    else
    {
        input.readObject("Indices", indices_data);
    }

    assignIndices(indices_data);
}

void BindIndexBuffer::write(Output& output) const
{
    Command::write(output);

    // write indices data
    if (output.version_greater_equal(0, 1, 4))
    {
        if (indices)
            output.writeObject("indices", indices->data);
        else
            output.writeObject("indices", nullptr);
    }
    else
    {
        if (indices)
            output.writeObject("Indices", indices->data);
        else
            output.writeObject("Indices", nullptr);
    }
}

void BindIndexBuffer::compile(Context& context)
{
    // nothing to compile
    if (!indices) return;

    // check if already compiled
    if (!indices->requiresCopy(context.deviceID))
    {
        return;
    }

    if (createBufferAndTransferData(context, {indices}, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE))
        indexType = computeIndexType(indices->data);
}

void BindIndexBuffer::record(CommandBuffer& commandBuffer) const
{
    vkCmdBindIndexBuffer(commandBuffer, indices->buffer->vk(commandBuffer.deviceID), indices->offset, indexType);
}
