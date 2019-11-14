/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/raytracing/DescriptorAccelerationStructure.h>

using namespace vsg;

DescriptorAccelerationStructure::DescriptorAccelerationStructure() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV)
{
}

DescriptorAccelerationStructure::DescriptorAccelerationStructure(const AccelerationStructures& accelerationStructures, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _accelerationStructures(accelerationStructures)
{
}

void DescriptorAccelerationStructure::read(Input& input)
{
    _accelerationStructures.clear();

    Descriptor::read(input);
}

void DescriptorAccelerationStructure::write(Output& output) const
{
    Descriptor::write(output);
}

void DescriptorAccelerationStructure::compile(Context& context)
{
    // check if we have already compiled the imageData.
    if (_vkAccelerationStructures.size() == _accelerationStructures.size()) return;

    for (const auto& accelstruct : _accelerationStructures)
    {
        accelstruct->compile(context);
        _vkAccelerationStructures.push_back(*accelstruct);
    }

    _descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    _descriptorAccelerationStructureInfo.accelerationStructureCount = static_cast<uint32_t>(_vkAccelerationStructures.size());
    _descriptorAccelerationStructureInfo.pAccelerationStructures = _vkAccelerationStructures.data();
    _descriptorAccelerationStructureInfo.pNext = nullptr;
}

bool DescriptorAccelerationStructure::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    Descriptor::assignTo(wds, descriptorSet);

    wds.descriptorCount = static_cast<uint32_t>(_vkAccelerationStructures.size()); // is this correct??
    wds.pNext = &_descriptorAccelerationStructureInfo;

    return true;
}

uint32_t DescriptorAccelerationStructure::getNumDescriptors() const
{
    return static_cast<uint32_t>(_accelerationStructures.size());
}
