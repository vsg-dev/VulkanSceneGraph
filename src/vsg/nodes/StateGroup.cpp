/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/StateGroup.h>

#include <vsg/io/Input.h>
#include <vsg/io/Output.h>

using namespace vsg;

StateSet::StateSet(Allocator* allocator) :
    Inherit(allocator)
{
}

StateSet::~StateSet()
{
}

void StateSet::read(Input& input)
{
    Object::read(input);

    _stateComponents.resize(input.readValue<uint32_t>("NumStateComponents"));
    for (auto& child : _stateComponents)
    {
        child = input.readObject<StateComponent>("StateComponent");
    }
}

void StateSet::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("NumStateComponents", _stateComponents.size());
    for (auto& child : _stateComponents)
    {
        output.writeObject("StateComponent", child.get());
    }
}

StateGroup::StateGroup(Allocator* allocator) :
    Inherit(allocator),
    _stateset(new StateSet)
{
}

StateGroup::StateGroup(StateSet* stateset) :
    Inherit(stateset ? stateset->getAllocator() : 0),
    _stateset(stateset)
{
}

StateGroup::~StateGroup()
{
}

void StateGroup::read(Input& input)
{
    Group::read(input);

    _stateset = input.readObject<StateSet>("StateSet");
}

void StateGroup::write(Output& output) const
{
    Group::write(output);

    output.writeObject("StateSet", _stateset.get());
}
