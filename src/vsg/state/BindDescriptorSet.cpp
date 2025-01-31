/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/vk/Context.h>

using namespace vsg;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BindDescriptorSets
//
BindDescriptorSets::BindDescriptorSets() :
    Inherit(1), // slot 1
    pipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
    firstSet(0)
{
}

BindDescriptorSets::BindDescriptorSets(const BindDescriptorSets& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    pipelineBindPoint(rhs.pipelineBindPoint),
    layout(copyop(rhs.layout)),
    firstSet(rhs.firstSet),
    descriptorSets(copyop(rhs.descriptorSets)),
    dynamicOffsets(rhs.dynamicOffsets)
{
}

int BindDescriptorSets::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(pipelineBindPoint, rhs.pipelineBindPoint))) return result;
    if ((result = compare_pointer(layout, rhs.layout))) return result;
    if ((result = compare_value(firstSet, rhs.firstSet))) return result;
    return compare_pointer_container(descriptorSets, rhs.descriptorSets);
}

void BindDescriptorSets::read(Input& input)
{
    _vulkanData.clear();

    StateCommand::read(input);

    if (input.version_greater_equal(0, 5, 4))
    {
        input.readValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    }

    input.readObject("layout", layout);
    input.read("firstSet", firstSet);
    input.readObjects("descriptorSets", descriptorSets);

    if (input.version_greater_equal(0, 5, 4))
    {
        input.readValues("dynamicOffsets", dynamicOffsets);
    }
}

void BindDescriptorSets::write(Output& output) const
{
    StateCommand::write(output);

    if (output.version_greater_equal(0, 5, 4))
    {
        output.writeValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    }

    output.writeObject("layout", layout);
    output.write("firstSet", firstSet);
    output.writeObjects("descriptorSets", descriptorSets);

    if (output.version_greater_equal(0, 5, 4))
    {
        output.writeValues("dynamicOffsets", dynamicOffsets);
    }
}

void BindDescriptorSets::compile(Context& context)
{
    auto& vkd = _vulkanData[context.deviceID];

    // no need to compile if already compiled
    if (vkd._vkPipelineLayout != 0 && vkd._vkDescriptorSets.size() == descriptorSets.size()) return;

    layout->compile(context);
    vkd._vkPipelineLayout = layout->vk(context.deviceID);

    vkd._vkDescriptorSets.resize(descriptorSets.size());
    for (size_t i = 0; i < descriptorSets.size(); ++i)
    {
        descriptorSets[i]->compile(context);
        vkd._vkDescriptorSets[i] = descriptorSets[i]->vk(context.deviceID);
    }
}

void BindDescriptorSets::record(CommandBuffer& commandBuffer) const
{
    //info("BindDescriptorSets::record() ", dynamicOffsets.size(), ", ", dynamicOffsets.data());
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, vkd._vkPipelineLayout, firstSet,
                            static_cast<uint32_t>(vkd._vkDescriptorSets.size()), vkd._vkDescriptorSets.data(),
                            static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BindDescriptorSet
//
BindDescriptorSet::BindDescriptorSet() :
    pipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
    firstSet(0)
{
}

BindDescriptorSet::BindDescriptorSet(const BindDescriptorSet& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    pipelineBindPoint(rhs.pipelineBindPoint),
    layout(copyop(rhs.layout)),
    firstSet(rhs.firstSet),
    descriptorSet(copyop(rhs.descriptorSet)),
    dynamicOffsets(rhs.dynamicOffsets)
{
}

int BindDescriptorSet::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(pipelineBindPoint, rhs.pipelineBindPoint))) return result;
    if ((result = compare_pointer(layout, rhs.layout))) return result;
    if ((result = compare_value(firstSet, rhs.firstSet))) return result;
    return compare_pointer(descriptorSet, rhs.descriptorSet);
}

void BindDescriptorSet::read(Input& input)
{
    _vulkanData.clear();

    StateCommand::read(input);

    if (input.version_greater_equal(0, 5, 4))
    {
        input.readValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    }

    input.readObject("layout", layout);
    input.read("firstSet", firstSet);
    input.readObject("descriptorSet", descriptorSet);

    if (input.version_greater_equal(0, 5, 4))
    {
        input.readValues("dynamicOffsets", dynamicOffsets);
    }
}

void BindDescriptorSet::write(Output& output) const
{
    StateCommand::write(output);

    if (output.version_greater_equal(0, 5, 4))
    {
        output.writeValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    }

    output.writeObject("layout", layout);
    output.write("firstSet", firstSet);
    output.writeObject("descriptorSet", descriptorSet);

    if (output.version_greater_equal(0, 5, 4))
    {
        output.writeValues("dynamicOffsets", dynamicOffsets);
    }
}

void BindDescriptorSet::compile(Context& context)
{
    auto& vkd = _vulkanData[context.deviceID];

    // no need to compile if already compiled
    if (vkd._vkPipelineLayout != 0 && vkd._vkDescriptorSet != 0) return;

    layout->compile(context);
    descriptorSet->compile(context);

    vkd._vkPipelineLayout = layout->vk(context.deviceID);
    vkd._vkDescriptorSet = descriptorSet->vk(context.deviceID);
}

void BindDescriptorSet::record(CommandBuffer& commandBuffer) const
{
    //info("BindDescriptorSet::record() ", dynamicOffsets.size(), ", ", dynamicOffsets.data());
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, vkd._vkPipelineLayout, firstSet,
                            1, &(vkd._vkDescriptorSet),
                            static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
}
