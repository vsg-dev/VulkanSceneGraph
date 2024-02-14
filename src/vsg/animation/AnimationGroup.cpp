/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/core/CopyOp.h>
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

ref_ptr<Object> AnimationGroup::clone(CopyOp& copyop) const
{
    auto new_ag = AnimationGroup::create();
    new_ag->animations = copyop(animations);
    new_ag->children = copyop(children);

    return new_ag;
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
