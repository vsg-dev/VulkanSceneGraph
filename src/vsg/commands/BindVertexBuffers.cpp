/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/io/Options.h>

using namespace vsg;

BindVertexBuffers::~BindVertexBuffers()
{
    for (auto& vkd : _vulkanData)
    {
        size_t numBufferEntries = std::min(vkd.buffers.size(), vkd.offsets.size());
        for (size_t i = 0; i < numBufferEntries; ++i)
        {
            if (vkd.buffers[i])
            {
                vkd.buffers[i]->release(vkd.offsets[i], 0); // TODO
            }
        }
    }
}

void BindVertexBuffers::add(ref_ptr<Buffer> buffer, VkDeviceSize offset)
{
    // assign to the appropriate compiledData for the buffer Device
    auto& vkd = _vulkanData[buffer->getDevice()->deviceID];

    vkd.buffers.push_back(buffer);
    vkd.vkBuffers.push_back(*buffer);
    vkd.offsets.push_back(offset);
}

void BindVertexBuffers::read(Input& input)
{
    Command::read(input);

    // clear Vulkan objects
    _vulkanData.clear();

    // read vertex arrays
    _arrays.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : _arrays)
    {
        input.readObject("Array", array);
    }
}

void BindVertexBuffers::write(Output& output) const
{
    Command::write(output);

    output.writeValue<uint32_t>("NumArrays", _arrays.size());
    for (auto& array : _arrays)
    {
        output.writeObject("Array", array.get());
    }
}

void BindVertexBuffers::compile(Context& context)
{
    // nothing to compile
    if (_arrays.empty()) return;

    auto& vkd = _vulkanData[context.deviceID];

    // already compiled
    if (vkd.buffers.size() == _arrays.size()) return;

    vkd.buffers.clear();
    vkd.vkBuffers.clear();
    vkd.offsets.clear();

    auto bufferDataList = vsg::createBufferAndTransferData(context, _arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    for (auto& bufferData : bufferDataList)
    {
        vkd.buffers.push_back(bufferData._buffer);
        vkd.vkBuffers.push_back(*(bufferData._buffer));
        vkd.offsets.push_back(bufferData._offset);
    }
}

void BindVertexBuffers::record(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindVertexBuffers(commandBuffer, _firstBinding, static_cast<uint32_t>(vkd.vkBuffers.size()), vkd.vkBuffers.data(), vkd.offsets.data());
}
