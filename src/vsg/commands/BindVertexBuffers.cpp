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

void BindVertexBuffers::read(Input& input)
{
    Command::read(input);

    // clear Vulkan objects
    _vulkanData.clear();

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("firstBinding", firstBinding);
        input.read("arrays", arrays);
    }
    else
    {
        // read vertex arrays
        arrays.resize(input.readValue<uint32_t>("NumArrays"));
        for (auto& array : arrays)
        {
            input.readObject("Array", array);
        }
    }
}

void BindVertexBuffers::write(Output& output) const
{
    Command::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("firstBinding", firstBinding);
        output.write("arrays", arrays);
    }
    else
    {
        output.writeValue<uint32_t>("NumArrays", arrays.size());
        for (auto& array : arrays)
        {
            output.writeObject("Array", array.get());
        }
    }
}

void BindVertexBuffers::compile(Context& context)
{
    // nothing to compile
    if (arrays.empty()) return;

    auto& vkd = _vulkanData[context.deviceID];

    // already compiled
    if (vkd.buffers.size() == arrays.size()) return;

    vkd.buffers.clear();
    vkd.vkBuffers.clear();
    vkd.offsets.clear();

    auto bufferDataList = vsg::createBufferAndTransferData(context, arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    for (auto& bufferData : bufferDataList)
    {
        vkd.buffers.push_back(bufferData.buffer);
        vkd.vkBuffers.push_back(bufferData.buffer->vk(context.deviceID));
        vkd.offsets.push_back(bufferData.offset);
    }
}

void BindVertexBuffers::record(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, static_cast<uint32_t>(vkd.vkBuffers.size()), vkd.vkBuffers.data(), vkd.offsets.data());
}
