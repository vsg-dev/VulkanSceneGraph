/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/state/ResourceHints.h>

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

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("maxSlot", maxSlot);
        input.read("numDescriptorSets", numDescriptorSets);
    }
    else
    {
        input.read("MaxSlot", maxSlot);
        input.read("NumDescriptorSets", numDescriptorSets);
    }

    descriptorPoolSizes.resize(input.readValue<uint32_t>("NumDescriptorPoolSize"));
    for (auto& [type, count] : descriptorPoolSizes)
    {
        input.readValue<uint32_t>("type", type);
        input.read("count", count);
    }

    if (input.version_greater_equal(0, 1, 11))
    {
        input.readValue<uint64_t>("minimumBufferSize", minimumBufferSize);
        input.readValue<uint64_t>("minimumBufferDeviceMemorySize", minimumBufferDeviceMemorySize);
        input.readValue<uint64_t>("minimumImageDeviceMemorySize", minimumImageDeviceMemorySize);
    }
}

void ResourceHints::write(Output& output) const
{
    Object::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("maxSlot", maxSlot);
        output.write("numDescriptorSets", numDescriptorSets);
    }
    else
    {
        output.write("MaxSlot", maxSlot);
        output.write("NumDescriptorSets", numDescriptorSets);
    }

    output.writeValue<uint32_t>("NumDescriptorPoolSize", descriptorPoolSizes.size());
    for (auto& [type, count] : descriptorPoolSizes)
    {
        output.writeValue<uint32_t>("type", type);
        output.write("count", count);
    }

    if (output.version_greater_equal(0, 1, 11))
    {
        output.writeValue<uint64_t>("minimumBufferSize", minimumBufferSize);
        output.writeValue<uint64_t>("minimumBufferDeviceMemorySize", minimumBufferDeviceMemorySize);
        output.writeValue<uint64_t>("minimumImageDeviceMemorySize", minimumImageDeviceMemorySize);
    }
}
