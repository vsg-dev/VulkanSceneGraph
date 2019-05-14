/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/CommandBuffer.h>

#include <vsg/traversals/CompileTraversal.h>

using namespace vsg;

void BindVertexBuffers::read(Input& input)
{
    Command::read(input);

    // clear Vulkan objects
    _buffers.clear();
    _vkBuffers.clear();
    _offsets.clear();

    // read vertex arrays
    _arrays.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : _arrays)
    {
        array = input.readObject<Data>("Array");
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

void BindVertexBuffers::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindVertexBuffers(commandBuffer, _firstBinding, static_cast<uint32_t>(_buffers.size()), _vkBuffers.data(), _offsets.data());
}

void BindVertexBuffers::compile(Context& context)
{
    // nothing to compile
    if (_arrays.empty()) return;

    // already compiled
    if (_buffers.size()==_arrays.size()) return;

    _buffers.clear();
    _vkBuffers.clear();
    _offsets.clear();

    auto bufferDataList = vsg::createBufferAndTransferData(context, _arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    for (auto& bufferData : bufferDataList)
    {
        _buffers.push_back(bufferData._buffer);
        _vkBuffers.push_back(*(bufferData._buffer));
        _offsets.push_back(bufferData._offset);
    }
}
