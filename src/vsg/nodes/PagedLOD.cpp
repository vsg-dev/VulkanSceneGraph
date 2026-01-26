/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/nodes/PagedLOD.h>

using namespace vsg;

PagedLOD::PagedLOD()
{
}

PagedLOD::PagedLOD(const PagedLOD& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    filename(rhs.filename),
    bound(rhs.bound)
{
    children[0].minimumScreenHeightRatio = rhs.children[0].minimumScreenHeightRatio;
    children[0].node = copyop(rhs.children[0].node);
    children[1].minimumScreenHeightRatio = rhs.children[1].minimumScreenHeightRatio;
    children[1].node = copyop(rhs.children[1].node);
}

PagedLOD::~PagedLOD()
{
}

int PagedLOD::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(bound, rhs.bound)) != 0) return result;

    // compare the children vector
    auto rhs_itr = rhs.children.begin();
    for (auto lhs_itr = children.begin(); lhs_itr != children.end(); ++lhs_itr, ++rhs_itr)
    {
        if ((result = compare_value(lhs_itr->minimumScreenHeightRatio, rhs_itr->minimumScreenHeightRatio)) != 0) return result;
        if ((result = compare_pointer(lhs_itr->node, rhs_itr->node)) != 0) return result;
    }

    return compare_value(filename, rhs.filename);
}

void PagedLOD::read(Input& input)
{
    Node::read(input);

    input.read("bound", bound);

    input.read("child.minimumScreenHeightRatio", children[0].minimumScreenHeightRatio);
    input.read("child.filename", filename);
    children[0].node = nullptr;

    if (input.filename)
    {
        auto path = filePath(input.filename);
        if (path)
        {
            filename = (path / filename).lexically_normal();
        }
    }

    input.read("child.minimumScreenHeightRatio", children[1].minimumScreenHeightRatio);
    input.read("child.node", children[1].node);

    options = Options::create_if(input.options, *input.options);
}

void PagedLOD::write(Output& output) const
{
    Node::write(output);

    output.write("bound", bound);

    output.write("child.minimumScreenHeightRatio", children[0].minimumScreenHeightRatio);
    output.write("child.filename", filename);

    output.write("child.minimumScreenHeightRatio", children[1].minimumScreenHeightRatio);
    output.write("child.node", children[1].node);
}
