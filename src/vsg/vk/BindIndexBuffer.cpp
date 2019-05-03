/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/State.h>

using namespace vsg;

void BindIndexBuffer::read(Input& input)
{
    Command::read(input);

    // reset the Vulkan related objects
    _bufferData._buffer = 0;
    _bufferData._offset = 0;
    _bufferData._range = 0;

    // read the key indices data
    _bufferData._data = input.readObject<Data>("Indices");
}

void BindIndexBuffer::write(Output& output) const
{
    Command::write(output);

    // write indices data
    output.writeObject("Indices", _bufferData._data.get());
}

void BindIndexBuffer::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindIndexBuffer(commandBuffer, *_bufferData._buffer, _bufferData._offset, _indexType);
}

void BindIndexBuffer::compile(Context& context)
{
    // check if already compiled
    if (_bufferData._buffer) return;

    auto bufferDataList = vsg::createBufferAndTransferData(context, {_bufferData._data}, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    if (!bufferDataList.empty())
    {
        _bufferData = bufferDataList.back();
    }
}
