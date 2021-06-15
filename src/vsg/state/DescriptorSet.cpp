/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/traversals/CompileTraversal.h>
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
}

void DescriptorSet::read(Input& input)
{
    Object::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("setLayout", setLayout);
        input.read("descriptors", descriptors);
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
        output.write("descriptors", descriptors);
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

#if USE_MUTEX
        std::scoped_lock<std::mutex> lock(context.descriptorPool->getMutex());
#endif
        _implementation[context.deviceID] = DescriptorSet::Implementation::create(context.device, context.descriptorPool, setLayout);
        _implementation[context.deviceID]->assign(context, descriptors);
    }
}

DescriptorSet::Implementation::Implementation(Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout) :
    _device(device),
    _descriptorPool(descriptorPool),
    _descriptorSetLayout(descriptorSetLayout)
{
    VkDescriptorSetLayout vkdescriptorSetLayout = descriptorSetLayout->vk(device->deviceID);

    VkDescriptorSetAllocateInfo descriptSetAllocateInfo = {};
    descriptSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptSetAllocateInfo.descriptorSetCount = 1;
    descriptSetAllocateInfo.pSetLayouts = &vkdescriptorSetLayout;

    if (VkResult result = vkAllocateDescriptorSets(*device, &descriptSetAllocateInfo, &_descriptorSet); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create DescriptorSet.", result};
    }
}

DescriptorSet::Implementation::~Implementation()
{
    if (_descriptorSet)
    {
#if USE_MUTEX
        std::scoped_lock<std::mutex> lock(_descriptorPool->getMutex());
#endif
        vkFreeDescriptorSets(*_device, *_descriptorPool, 1, &_descriptorSet);
    }
}

void DescriptorSet::Implementation::assign(Context& context, const Descriptors& descriptors)
{
    // should we doing anything about previous _descriptor that may have been assigned?
    _descriptors = descriptors;

    if (_descriptors.empty()) return;

    VkWriteDescriptorSet* descriptorWrites = context.scratchMemory->allocate<VkWriteDescriptorSet>(_descriptors.size());

    for (size_t i = 0; i < _descriptors.size(); ++i)
    {
        descriptors[i]->assignTo(context, descriptorWrites[i]);
        descriptorWrites[i].dstSet = _descriptorSet;
    }

    vkUpdateDescriptorSets(*_device, static_cast<uint32_t>(descriptors.size()), descriptorWrites, 0, nullptr);

    // clean up scratch memory so it can be reused.
    context.scratchMemory->release();
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

void BindDescriptorSets::read(Input& input)
{
    _vulkanData.clear();

    Object::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("layout", layout);
        input.read("firstSet", firstSet);
        input.read("descriptorSets", descriptorSets);
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
    Object::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("layout", layout);
        output.write("firstSet", firstSet);
        output.write("descriptorSets", descriptorSets);
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
