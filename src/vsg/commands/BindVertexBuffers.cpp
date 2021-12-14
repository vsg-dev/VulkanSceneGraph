/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/io/Options.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

BindVertexBuffers::BindVertexBuffers(uint32_t in_firstBinding, const DataList& in_arrays) :
    firstBinding(in_firstBinding)
{
    assignArrays(in_arrays);
}

BindVertexBuffers::~BindVertexBuffers()
{
}

void BindVertexBuffers::assignArrays(const DataList& arrayData)
{
    arrays.clear();
    arrays.reserve(arrayData.size());
    for (auto& data : arrayData)
    {
        arrays.push_back(BufferInfo::create(data));
    }
}

void BindVertexBuffers::read(Input& input)
{
    Command::read(input);

    // clear Vulkan objects
    _vulkanData.clear();

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("firstBinding", firstBinding);
    }

    // read vertex arrays
    DataList dataList;
    dataList.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : dataList)
    {
        input.readObject("Array", array);
    }
    assignArrays(dataList);
}

void BindVertexBuffers::write(Output& output) const
{
    Command::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("firstBinding", firstBinding);
    }

    output.writeValue<uint32_t>("NumArrays", arrays.size());
    for (auto& array : arrays)
    {
        if (array)
            output.writeObject("Array", array->data.get());
        else
            output.writeObject("Array", nullptr);
    }
}

void BindVertexBuffers::compile(Context& context)
{
    // nothing to compile
    if (arrays.empty()) return;

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

    if (!requiresCreateAndCopy)
    {
        return;
    }

    auto& vkd = _vulkanData[context.deviceID];

    vkd.vkBuffers.clear();
    vkd.offsets.clear();

    if (createBufferAndTransferData(context, arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE))
    {
        for (auto& bufferInfo : arrays)
        {
            vkd.vkBuffers.push_back(bufferInfo->buffer->vk(context.deviceID));
            vkd.offsets.push_back(bufferInfo->offset);
        }
    }
}

void BindVertexBuffers::record(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, static_cast<uint32_t>(vkd.vkBuffers.size()), vkd.vkBuffers.data(), vkd.offsets.data());
}
