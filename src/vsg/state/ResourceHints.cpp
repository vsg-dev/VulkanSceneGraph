/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/ResourceHints.h>

using namespace vsg;

ResourceHints::ResourceHints()
{
}

ResourceHints::~ResourceHints()
{
}

void ResourceHints::read(Input& input)
{
    Object::read(input);

    input.read("maxSlot", maxSlot);
    input.read("numDescriptorSets", numDescriptorSets);

    if (input.version_greater_equal(0, 7, 3))
        descriptorPoolSizes.resize(input.readValue<uint32_t>("descriptorPoolSizes"));
    else
        descriptorPoolSizes.resize(input.readValue<uint32_t>("NumDescriptorPoolSize"));

    for (auto& [type, count] : descriptorPoolSizes)
    {
        input.readValue<uint32_t>("type", type);
        input.read("count", count);
    }

    input.read("minimumBufferSize", minimumBufferSize);
    input.read("minimumDeviceMemorySize", minimumDeviceMemorySize);

    if (input.version_greater_equal(1, 1, 8))
    {
        input.read("minimumStagingBufferSize", minimumStagingBufferSize);
    }

    if (input.version_greater_equal(1, 0, 10))
    {
        input.read("numLightsRange", numLightsRange);
        input.read("numShadowMapsRange", numShadowMapsRange);
        input.read("shadowMapSize", shadowMapSize);
    }

    if (input.version_greater_equal(1, 1, 8))
    {
        input.read("numDatabasePagerReadThreads", numDatabasePagerReadThreads);
        input.readValue<uint32_t>("dataTransferHint", dataTransferHint);
    }
}

void ResourceHints::write(Output& output) const
{
    Object::write(output);

    output.write("maxSlot", maxSlot);
    output.write("numDescriptorSets", numDescriptorSets);

    if (output.version_greater_equal(0, 7, 3))
        output.writeValue<uint32_t>("descriptorPoolSizes", descriptorPoolSizes.size());
    else
        output.writeValue<uint32_t>("NumDescriptorPoolSize", descriptorPoolSizes.size());

    for (auto& [type, count] : descriptorPoolSizes)
    {
        output.writeValue<uint32_t>("type", type);
        output.write("count", count);
    }

    output.write("minimumBufferSize", minimumBufferSize);
    output.write("minimumDeviceMemorySize", minimumDeviceMemorySize);

    if (output.version_greater_equal(1, 1, 8))
    {
        output.write("minimumStagingBufferSize", minimumStagingBufferSize);
    }

    if (output.version_greater_equal(1, 0, 10))
    {
        output.write("numLightsRange", numLightsRange);
        output.write("numShadowMapsRange", numShadowMapsRange);
        output.write("shadowMapSize", shadowMapSize);
    }

    if (output.version_greater_equal(1, 1, 8))
    {
        output.write("numDatabasePagerReadThreads", numDatabasePagerReadThreads);
        output.writeValue<uint32_t>("dataTransferHint", dataTransferHint);
    }
}
