/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/nodes/PagedLOD.h>

using namespace vsg;

PagedLOD::PagedLOD(Allocator* allocator) :
    Inherit(allocator)
{
}

PagedLOD::~PagedLOD()
{
}

void PagedLOD::read(Input& input)
{
    Node::read(input);

    input.read("Bound", _bound);

    input.read("maxSlot", _maxSlot);

    _descriptorPoolSizes.resize(input.readValue<uint32_t>("NumDescriptorPoolSize"));
    for (auto& [type, count] : _descriptorPoolSizes)
    {
        input.readValue<uint32_t>("type", type);
        input.read("count", count);
    }

    input.read("MinimumScreenHeightRatio", _children[0].minimumScreenHeightRatio);
    input.read("Filename", filename);
    _children[0].node = nullptr;

    input.read("MinimumScreenHeightRatio", _children[1].minimumScreenHeightRatio);
    _children[1].node = input.readObject<Node>("Child");
}

void PagedLOD::write(Output& output) const
{
    Node::write(output);

    output.write("Bound", _bound);

    output.write("maxSlot", _maxSlot);

    output.writeValue<uint32_t>("NumDescriptorPoolSize", _descriptorPoolSizes.size());
    for (auto& [type, count] : _descriptorPoolSizes)
    {
        output.writeValue<uint32_t>("type", type);
        output.write("count", count);
    }

    output.write("MinimumScreenHeightRatio", _children[0].minimumScreenHeightRatio);
    output.write("Filename", filename);

    output.write("MinimumScreenHeightRatio", _children[1].minimumScreenHeightRatio);
    output.writeObject("Child", _children[1].node);
}
