/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Switch.h>

#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>

using namespace vsg;

Switch::Switch(Allocator* allocator) :
    Inherit(allocator)
{
}

Switch::~Switch()
{
}

void Switch::read(Input& input)
{
    Node::read(input);

    if (input.version_greater_equal(0, 2, 2))
    {
        children.resize(input.readValue<uint32_t>("children"));
        for (auto& child : children)
        {
            input.read("child.mask", child.mask);
            input.read("child.node", child.node);
        }
    }
    else if (input.version_greater_equal(0, 1, 4))
    {
        children.resize(input.readValue<uint32_t>("children"));
        for (auto& child : children)
        {
            bool enabled;
            input.read("child.enabled", enabled);
            input.read("child.node", child.node);
            child.mask = enabled ? 0 : 0xffffffff;
        }
    }
    else
    {
        children.resize(input.readValue<uint32_t>("NumChildren"));
        for (auto& child : children)
        {
            bool enabled;
            input.read("enabled", enabled);
            input.read("node", child.node);
            child.mask = enabled ? 0 : 0xffffffff;
        }
    }
}

void Switch::write(Output& output) const
{
    Node::write(output);

    if (output.version_greater_equal(0, 2, 2))
    {
        output.writeValue<uint32_t>("children", children.size());
        for (auto& child : children)
        {
            output.write("child.mask", child.mask);
            output.write("child.node", child.node);
        }
    }
    else if (output.version_greater_equal(0, 1, 4))
    {
        output.writeValue<uint32_t>("children", children.size());
        for (auto& child : children)
        {
            bool enabled = child.mask == 0 ? false : true;
            output.write("child.enabled", enabled);
            output.write("child.node", child.node);
        }
    }
    else
    {
        output.writeValue<uint32_t>("NumChildren", children.size());
        for (auto& child : children)
        {
            bool enabled = child.mask == 0 ? false : true;
            output.write("enabled", enabled);
            output.write("node", child.node);
        }
    }
}

void Switch::addChild(uint32_t mask, ref_ptr<Node> child)
{
    children.push_back(Child{mask, child});
}

void Switch::addChild(bool enabled, ref_ptr<Node> child)
{
    children.push_back(Child{boolToMask(enabled), child});
}

void Switch::setAllChildren(bool enabled)
{
    uint32_t mask = boolToMask(enabled);
    for (auto& child : children) child.mask = mask;
}

void Switch::setSingleChildOn(size_t index)
{
    for (size_t i = 0; i < children.size(); ++i)
    {
        children[i].mask = boolToMask(i == index);
    }
}
