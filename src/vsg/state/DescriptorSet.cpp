/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/viewer/View.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

#define USE_MUTEX 0

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::DescriptorSet(ref_ptr<DescriptorSetLayout> in_descriptorSetLayout, const Descriptors& in_descriptors) :
    setLayout(in_descriptorSetLayout),
    descriptors(in_descriptors)
{
}

DescriptorSet::~DescriptorSet()
{
    release();
}

int DescriptorSet::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer(setLayout, rhs.setLayout))) return result;
    return compare_pointer_container(descriptors, rhs.descriptors);
}

void DescriptorSet::read(Input& input)
{
    Object::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("setLayout", setLayout);
        input.readObjects("descriptors", descriptors);
    }
    else
    {
        input.read("DescriptorSetLayout", setLayout);

        descriptors.resize(input.readValue<uint32_t>("NumDescriptors"));
        for (auto& descriptor : descriptors)
        {
            input.read("Descriptor", descriptor);
        }
    }
}

void DescriptorSet::write(Output& output) const
{
    Object::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("setLayout", setLayout);
        output.writeObjects("descriptors", descriptors);
    }
    else
    {
        output.write("DescriptorSetLayout", setLayout);

        output.writeValue<uint32_t>("NumDescriptors", descriptors.size());
        for (auto& descriptor : descriptors)
        {
            output.write("Descriptor", descriptor);
        }
    }
}

void DescriptorSet::compile(Context& context)
{
    if (!_implementation[context.deviceID])
    {
        // make sure all the contributing objects are compiled
        if (setLayout) setLayout->compile(context);
        for (auto& descriptor : descriptors) descriptor->compile(context);

#if 1
        _implementation[context.deviceID] = context.allocateDescriptorSet(setLayout);
        _implementation[context.deviceID]->assign(context, descriptors);
#else

#if USE_MUTEX
        std::scoped_lock<std::mutex> lock(context.descriptorPool->getMutex());
#endif
        _implementation[context.deviceID] = DescriptorSet::Implementation::create(context.descriptorPool, setLayout);
        _implementation[context.deviceID]->assign(context, descriptors);
#endif
    }
}

void DescriptorSet::release(uint32_t deviceID)
{
#if 1
    recyle(_implementation[deviceID]);
#else
    _implementation[deviceID] = {};
#endif
}
void DescriptorSet::release()
{
#if 1
    for(auto& dsi : _implementation) recyle(dsi);
#endif
    _implementation.clear();
}

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

int BindDescriptorSets::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(pipelineBindPoint, rhs.pipelineBindPoint))) return result;
    if ((result = compare_pointer(layout, rhs.layout))) return result;
    if ((result = compare_value(firstSet, rhs.firstSet))) return result;
    return compare_pointer_container(descriptorSets, rhs.descriptorSets);
}

void BindDescriptorSets::read(Input& input)
{
    _vulkanData.clear();

    if (input.version_greater_equal(0, 2, 13))
    {
        StateCommand::read(input);
    }
    else
    {
        Object::read(input);
    }

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("layout", layout);
        input.read("firstSet", firstSet);
        input.readObjects("descriptorSets", descriptorSets);
    }
    else
    {
        input.read("PipelineLayout", layout);

        input.read("firstSet", firstSet);

        descriptorSets.resize(input.readValue<uint32_t>("NumDescriptorSets"));
        for (auto& descriptorSet : descriptorSets)
        {
            input.read("DescriptorSets", descriptorSet);
        }
    }
}

void BindDescriptorSets::write(Output& output) const
{
    if (output.version_greater_equal(0, 2, 13))
    {
        StateCommand::write(output);
    }
    else
    {
        Object::write(output);
    }

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("layout", layout);
        output.write("firstSet", firstSet);
        output.writeObjects("descriptorSets", descriptorSets);
    }
    else
    {
        output.write("PipelineLayout", layout);
        output.write("firstSet", firstSet);

        output.writeValue<uint32_t>("NumDescriptorSets", descriptorSets.size());
        for (auto& descriptorSet : descriptorSets)
        {
            output.write("DescriptorSets", descriptorSet);
        }
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
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, vkd._vkPipelineLayout, firstSet, static_cast<uint32_t>(vkd._vkDescriptorSets.size()), vkd._vkDescriptorSets.data(), 0, nullptr);
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

int BindDescriptorSet::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(pipelineBindPoint, rhs.pipelineBindPoint))) return result;
    if ((result = compare_pointer(layout, rhs.layout))) return result;
    if ((result = compare_value(firstSet, rhs.firstSet))) return result;
    return compare_pointer(descriptorSet, rhs.descriptorSet);
}

void BindDescriptorSet::read(Input& input)
{
    _vulkanData.clear();

    StateCommand::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("layout", layout);
        input.read("firstSet", firstSet);
        input.read("descriptorSet", descriptorSet);
    }
    else
    {
        input.read("PipelineLayout", layout);
        input.read("firstSet", firstSet);
        input.read("DescriptorSet", descriptorSet);
    }
}

void BindDescriptorSet::write(Output& output) const
{
    StateCommand::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("layout", layout);
        output.write("firstSet", firstSet);
        output.write("descriptorSet", descriptorSet);
    }
    else
    {
        output.write("PipelineLayout", layout);
        output.write("firstSet", firstSet);
        output.write("DescriptorSet", descriptorSet);
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
    auto& vkd = _vulkanData[commandBuffer.deviceID];

    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, vkd._vkPipelineLayout, firstSet, 1, &(vkd._vkDescriptorSet), 0, nullptr);
}
