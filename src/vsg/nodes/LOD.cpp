/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/nodes/LOD.h>
#include <vsg/io/Options.h>

using namespace vsg;

LOD::LOD(Allocator* allocator) :
    Inherit(allocator)
{
}

LOD::~LOD()
{
}

void LOD::read(Input& input)
{
    Node::read(input);

    input.read("Bound", _bound);

    _children.resize(input.readValue<uint32_t>("NumChildren"));
    for (auto& child : _children)
    {
        input.read("MinimumScreenHeightRatio", child.minimumScreenHeightRatio);
        input.readObject("Child", child.node);
    }
}

void LOD::write(Output& output) const
{
    Node::write(output);

    output.write("Bound", _bound);

    output.writeValue<uint32_t>("NumChildren", _children.size());
    for (auto& child : _children)
    {
        output.write("MinimumScreenHeightRatio", child.minimumScreenHeightRatio);
        output.writeObject("Child", child.node);
    }
}
