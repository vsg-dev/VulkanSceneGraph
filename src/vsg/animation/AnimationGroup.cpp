/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>

using namespace vsg;

AnimationGroup::AnimationGroup(size_t numChildren) :
    Inherit(numChildren)
{
}

AnimationGroup::~AnimationGroup()
{
}

int AnimationGroup::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer_container(animations, rhs.animations)) != 0) return result;
    return compare_pointer_container(children, rhs.children);
}

void AnimationGroup::read(Input& input)
{
    Node::read(input);

    input.readObjects("animations", animations);
    input.readObjects("children", children);
}

void AnimationGroup::write(Output& output) const
{
    Node::write(output);

    output.writeObjects("animations", animations);
    output.writeObjects("children", children);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AnimationTransform
//
AnimationTransform::AnimationTransform() :
    Inherit()
{
}

AnimationTransform::~AnimationTransform()
{
}

int AnimationTransform::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(name, rhs.name)) != 0) return result;
    if ((result = compare_pointer(matrix, rhs.matrix)) != 0) return result;
    return compare_pointer_container(children, rhs.children);
}

void AnimationTransform::read(Input& input)
{
    Node::read(input);

    input.read("name", name);
    input.read("matrix", matrix);
    input.readObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}

void AnimationTransform::write(Output& output) const
{
    Node::write(output);

    output.write("name", name);
    output.write("matrix", matrix);
    output.writeObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}

dmat4 AnimationTransform::transform(const dmat4& mv) const
{
    return mv * matrix->value();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// RiggedTransform
//
RiggedTransform::RiggedTransform() :
    Inherit()
{
}

RiggedTransform::~RiggedTransform()
{
}

int RiggedTransform::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(name, rhs.name)) != 0) return result;
    if ((result = compare_pointer(matrix, rhs.matrix)) != 0) return result;
    return compare_pointer_container(children, rhs.children);
}

void RiggedTransform::read(Input& input)
{
    Node::read(input);

    input.read("name", name);
    input.read("matrix", matrix);
    input.readObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}

void RiggedTransform::write(Output& output) const
{
    Node::write(output);

    output.write("name", name);
    output.write("matrix", matrix);
    output.writeObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}
