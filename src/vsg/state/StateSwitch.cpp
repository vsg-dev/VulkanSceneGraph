/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/StateSwitch.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

StateSwitch::StateSwitch()
{
}

StateSwitch::StateSwitch(const StateSwitch& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop)
{
    children.reserve(rhs.children.size());
    for (auto child : rhs.children)
    {
        children.push_back(Child{child.mask, copyop(child.stateCommand)});
    }
}

int StateSwitch::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    // compare the children vector
    if (children.size() < rhs.children.size()) return -1;
    if (children.size() > rhs.children.size()) return 1;
    if (children.empty()) return 0;

    auto rhs_itr = rhs.children.begin();
    for (auto lhs_itr = children.begin(); lhs_itr != children.end(); ++lhs_itr, ++rhs_itr)
    {
        if ((result = compare_value(lhs_itr->mask, rhs_itr->mask)) != 0) return result;
        if ((result = compare_pointer(lhs_itr->stateCommand, rhs_itr->stateCommand)) != 0) return result;
    }
    return 0;
}

void StateSwitch::compile(Context& context)
{
    for (auto& child : children) child.stateCommand->compile(context);
}

void StateSwitch::record(CommandBuffer& commandBuffer) const
{
    for (auto& child : children)
    {
        if ((commandBuffer.traversalMask & (commandBuffer.overrideMask | child.mask)) != MASK_OFF)
        {
            child.stateCommand->record(commandBuffer);
        }
    }
}

void StateSwitch::read(Input& input)
{
    StateCommand::read(input);

    children.resize(input.readValue<uint32_t>("children"));
    for (auto& child : children)
    {
        input.read("child.mask", child.mask);
        input.read("child.stateCommand", child.stateCommand);
    }
}

void StateSwitch::write(Output& output) const
{
    StateCommand::write(output);

    output.writeValue<uint32_t>("children", children.size());
    for (auto& child : children)
    {
        output.write("child.mask", child.mask);
        output.write("child.stateCommand", child.stateCommand);
    }
}
