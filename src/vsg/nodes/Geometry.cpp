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

#include <vsg/traversals/DispatchTraversal.h>

#include <vsg/nodes/Geometry.h>

#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Geometry node
//       vertex arrays
//       index arrays
//       draw + draw DrawIndexed
//
Geometry::Geometry(Allocator* allocator) :
    Inherit(allocator)
{
}

void Geometry::read(Input& input)
{
    Node::read(input);

    _arrays.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : _arrays)
    {
        array = input.readObject<Data>("Array");
    }

    _indices = input.readObject<Data>("Indices");

    _commands.resize(input.readValue<uint32_t>("NumCommands"));
    for (auto& command : _commands)
    {
        command = input.readObject<Command>("Command");
    }
}

void Geometry::write(Output& output) const
{
    Node::write(output);

    output.writeValue<uint32_t>("NumArrays", _arrays.size());
    for (auto& array : _arrays)
    {
        output.writeObject("Array", array.get());
    }

    output.writeObject("Indices", _indices.get());

    output.writeValue<uint32_t>("NumCommands", _commands.size());
    for (auto& command : _commands)
    {
        output.writeObject("Command", command.get());
    }
}

void Geometry::compile(Context& context)
{
    if (!_renderImplementation.empty()) return;

    _renderImplementation.clear();

    bool failure = false;

    if (_indices)
    {
        DataList dataList;
        dataList.reserve(_arrays.size() + 1);
        dataList.insert(dataList.end(), _arrays.begin(), _arrays.end());
        dataList.emplace_back(_indices);

        auto bufferData = vsg::createBufferAndTransferData(context, dataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
        if (!bufferData.empty())
        {
            BufferDataList vertexBufferData(bufferData.begin(), bufferData.begin() + _arrays.size());
            vsg::ref_ptr<vsg::BindVertexBuffers> bindVertexBuffers = vsg::BindVertexBuffers::create(0, vertexBufferData);
            if (bindVertexBuffers)
                _renderImplementation.emplace_back(bindVertexBuffers);
            else
                failure = true;

            vsg::ref_ptr<vsg::BindIndexBuffer> bindIndexBuffer = vsg::BindIndexBuffer::create(bufferData.back(), VK_INDEX_TYPE_UINT16);
            if (bindIndexBuffer)
                _renderImplementation.emplace_back(bindIndexBuffer);
            else
                failure = true;
        }
        else
            failure = true;
    }
    else
    {
        auto vertexBufferData = vsg::createBufferAndTransferData(context, _arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
        if (!vertexBufferData.empty())
        {
            vsg::ref_ptr<vsg::BindVertexBuffers> bindVertexBuffers = vsg::BindVertexBuffers::create(0, vertexBufferData);
            if (bindVertexBuffers)
                _renderImplementation.emplace_back(bindVertexBuffers);
            else
                failure = true;
        }
        else
            failure = true;
    }

    if (failure)
    {
        //std::cout<<"Failed to create required arrays/indices buffers on GPU."<<std::endl;
        _renderImplementation.clear();
        return;
    }

    // add the commands in the the _renderImplementation.
    _renderImplementation.insert(_renderImplementation.end(), _commands.begin(), _commands.end());
}

void Geometry::dispatch(CommandBuffer& commandBuffer) const
{
    for (auto& command : _renderImplementation)
    {
        command->dispatch(commandBuffer);
    }
}
