/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/Draw.h>

#include <vsg/io/ReaderWriter.h>

#include <vsg/traversals/DispatchTraversal.h>


#include <vsg/nodes/Geometry.h>

#include <iostream>

using namespace vsg;

//
//  Geometry node
//       vertex arrays
//       index arrays
//       draw + draw DrawIndexed
//       push constants for per geometry colours etc.
//
//       Maps to a Group containing StateGroup + Binds + DrawIndex/Draw etc + Push constants
//
/////////////////////////////////////////////////////////////////////////////////////////
//
// Geometry
//
Geometry::Geometry(Allocator* allocator) :
    Inherit(allocator)
{
}

void Geometry::accept(DispatchTraversal& dv) const
{
    if (_renderImplementation) _renderImplementation->accept(dv);
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
    auto vertexBufferData = vsg::createBufferAndTransferData(context.device, context.commandPool, context.graphicsQueue, _arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);

    _renderImplementation = new vsg::Group;

    // set up vertex buffer binding
    vsg::ref_ptr<vsg::BindVertexBuffers> bindVertexBuffers = vsg::BindVertexBuffers::create(0, vertexBufferData);  // device dependent
    _renderImplementation->addChild(bindVertexBuffers); // device dependent

    // set up index buffer binding
    if(_indices &&_indices->dataSize() > 0)
    {
        auto indexBufferData = vsg::createBufferAndTransferData(context.device, context.commandPool, context.graphicsQueue, { _indices }, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
        vsg::ref_ptr<vsg::BindIndexBuffer> bindIndexBuffer = vsg::BindIndexBuffer::create(indexBufferData.front(), VK_INDEX_TYPE_UINT16); // device dependent
        _renderImplementation->addChild(bindIndexBuffer); // device dependent
    }

    // add the commands in the the _renderImplementation group.
    for(auto& command : _commands) _renderImplementation->addChild(command);
}

