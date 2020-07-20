/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/ResourceHints.h>
#include <vsg/io/Options.h>

using namespace vsg;

ResourceHints::ResourceHints(Allocator* allocator) :
    Inherit(allocator)
{
}

ResourceHints::~ResourceHints()
{
}

void ResourceHints::read(Input& input)
{
    Object::read(input);

    input.read("MaxSlot", _maxSlot);
    input.read("NumDescriptorSets", _numDescriptorSets);

    _descriptorPoolSizes.resize(input.readValue<uint32_t>("NumDescriptorPoolSize"));
    for (auto& [type, count] : _descriptorPoolSizes)
    {
        input.readValue<uint32_t>("type", type);
        input.read("count", count);
    }
}

void ResourceHints::write(Output& output) const
{
    Object::write(output);

    output.write("MaxSlot", _maxSlot);
    output.write("NumDescriptorSets", _numDescriptorSets);

    output.writeValue<uint32_t>("NumDescriptorPoolSize", _descriptorPoolSizes.size());
    for (auto& [type, count] : _descriptorPoolSizes)
    {
        output.writeValue<uint32_t>("type", type);
        output.write("count", count);
    }
}
