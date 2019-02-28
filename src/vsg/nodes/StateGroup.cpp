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
    Inherit(allocator),
    _bindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
    _firstSet(0)
{
}

StateSet::~StateSet()
{
}

void StateSet::compile(Context& context)
{
    Descriptors descriptors;
    descriptors.reserve(_attributes.size());
    for(auto& attribute : _attributes)
    {
        auto descriptor = attribute->compile(context);
        if (descriptor) descriptors.push_back(descriptor);
    }

    // should descriptorSetLayouts be provided by StateSet? A shared instance?
    DescriptorSets descriptorSets{DescriptorSet::create(context.device, context.descriptorPool, context.descriptorSetLayouts, descriptors)};
    _bindDescriptorSets = BindDescriptorSets::create(_bindPoint, context.pipelineLayout, _firstSet, descriptorSets);
}

void StateSet::read(Input& input)
{
    Object::read(input);

    _attributes.resize(input.readValue<uint32_t>("NumStateAttributes"));
    for (auto& child : _attributes)
    {
        child = input.readObject<StateAttribute>("StateAttribute");
    }
}

void StateSet::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("NumStateAttributes", _attributes.size());
    for (auto& child : _attributes)
    {
        output.writeObject("StateAttribute", child.get());
    }
}

StateGroup::StateGroup(Allocator* allocator) :
    Inherit(allocator)
{
}

StateGroup::~StateGroup()
{
}

void StateGroup::read(Input& input)
{
    Group::read(input);

    _stateCommands.resize(input.readValue<uint32_t>("NumStateCommands"));
    for (auto& child : _stateCommands)
    {
        child = input.readObject<StateCommand>("StateCommand");
    }
}

void StateGroup::write(Output& output) const
{
    Group::write(output);

    output.writeValue<uint32_t>("NumStateCommands", _stateCommands.size());
    for (auto& child : _stateCommands)
    {
        output.writeObject("StateCommand", child.get());
    }
}

void StateGroup::compile(Context& context)
{
    for(auto& stateCommand : _stateCommands)
    {
        stateCommand->compile(context);
    }
}
