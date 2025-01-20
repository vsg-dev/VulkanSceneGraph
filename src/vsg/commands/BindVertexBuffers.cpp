/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/core/compare.h>
#include <vsg/vk/Context.h>

using namespace vsg;

BindVertexBuffers::BindVertexBuffers(uint32_t in_firstBinding, const DataList& in_arrays) :
    firstBinding(in_firstBinding)
{
    assignArrays(in_arrays);
}

BindVertexBuffers::~BindVertexBuffers()
{
}

int BindVertexBuffers::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(firstBinding, rhs.firstBinding))) return result;
    return compare_pointer_container(arrays, rhs.arrays);
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

    input.read("firstBinding", firstBinding);

    DataList dataList;
    dataList.resize(input.readValue<uint32_t>("arrays"));
    for (auto& array : dataList)
    {
        input.readObject("array", array);
    }
    assignArrays(dataList);
}

void BindVertexBuffers::write(Output& output) const
{
    Command::write(output);

    output.write("firstBinding", firstBinding);

    output.writeValue<uint32_t>("arrays", arrays.size());
    for (const auto& array : arrays)
    {
        if (array)
            output.writeObject("array", array->data.get());
        else
            output.writeObject("array", nullptr);
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

    if (requiresCreateAndCopy)
    {
        createBufferAndTransferData(context, arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    }

    assignVulkanArrayData(deviceID, arrays, _vulkanData[deviceID]);
}

void BindVertexBuffers::record(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, static_cast<uint32_t>(vkd.vkBuffers.size()), vkd.vkBuffers.data(), vkd.offsets.data());
}
