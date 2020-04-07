/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorSet.h>

#include <vsg/traversals/CompileTraversal.h>

using namespace vsg;

#define USE_MUTEX 0

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::DescriptorSet(ref_ptr<DescriptorSetLayout> descriptorSetLayout, const Descriptors& descriptors) :
    _descriptorSetLayout(descriptorSetLayout),
    _descriptors(descriptors)
{
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::read(Input& input)
{
    Object::read(input);

    input.readObject("DescriptorSetLayout", _descriptorSetLayout);

    _descriptors.resize(input.readValue<uint32_t>("NumDescriptors"));
    for (auto& descriptor : _descriptors)
    {
        input.readObject("Descriptor", descriptor);
    }
}

void DescriptorSet::write(Output& output) const
{
    Object::write(output);

    output.writeObject("DescriptorSetLayout", _descriptorSetLayout.get());

    output.writeValue<uint32_t>("NumDescriptors", _descriptors.size());
    for (auto& descriptor : _descriptors)
    {
        output.writeObject("Descriptor", descriptor.get());
    }
}

void DescriptorSet::compile(Context& context)
{
    if (!_implementation[context.deviceID])
    {
        // make sure all the contributing objects are compiled
        if (_descriptorSetLayout) _descriptorSetLayout->compile(context);
        for (auto& descriptor : _descriptors) descriptor->compile(context);

#if USE_MUTEX
        std::lock_guard<std::mutex> lock(context.descriptorPool->getMutex());
#endif
        _implementation[context.deviceID] = DescriptorSet::Implementation::create(context.device, context.descriptorPool, _descriptorSetLayout);
        _implementation[context.deviceID]->assign(context, _descriptors);
    }
}

DescriptorSet::Implementation::Implementation(VkDescriptorSet descriptorSet, Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout) :
    _descriptorSet(descriptorSet),
    _device(device),
    _descriptorPool(descriptorPool),
    _descriptorSetLayout(descriptorSetLayout)
{
}

DescriptorSet::Implementation::~Implementation()
{
    if (_descriptorSet)
    {
#if USE_MUTEX
        std::lock_guard<std::mutex> lock(_descriptorPool->getMutex());
#endif
        vkFreeDescriptorSets(*_device, *_descriptorPool, 1, &_descriptorSet);
    }
}

DescriptorSet::Implementation::Result DescriptorSet::Implementation::create(Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout)
{
    if (!device || !descriptorPool || !descriptorSetLayout)
    {
        return Result("Error: vsg::DescriptorPool::create(...) failed to create DescriptorPool, undefined Device, DescriptorPool or DescriptorSetLayout.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkDescriptorSetLayout vkdescriptorSetLayout = descriptorSetLayout->vk(device->deviceID);

    VkDescriptorSetAllocateInfo descriptSetAllocateInfo = {};
    descriptSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptSetAllocateInfo.descriptorSetCount = 1;
    descriptSetAllocateInfo.pSetLayouts = &vkdescriptorSetLayout;

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(*device, &descriptSetAllocateInfo, &descriptorSet);
    if (result == VK_SUCCESS)
    {
        return Result(new DescriptorSet::Implementation(descriptorSet, device, descriptorPool, descriptorSetLayout));
    }
    else
    {
        return Result("Error: Failed to create DescriptorSet.", result);
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
        _descriptors[i]->assignTo(context, descriptorWrites[i]);
        descriptorWrites[i].dstSet = _descriptorSet;
    }

    vkUpdateDescriptorSets(*_device, _descriptors.size(), descriptorWrites, 0, nullptr);

    // clean up scratch memory so it can be reused.
    context.scratchMemory->release();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BindDescriptorSets
//
BindDescriptorSets::BindDescriptorSets() :
    Inherit(1), // slot 1
    _bindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
    _firstSet(0)
{
}

void BindDescriptorSets::read(Input& input)
{
    _vulkanData.clear();

    Object::read(input);

    input.readObject("PipelineLayout", _pipelineLayout);

    input.read("firstSet", _firstSet);

    _descriptorSets.resize(input.readValue<uint32_t>("NumDescriptorSets"));
    for (auto& descriptorSet : _descriptorSets)
    {
        input.readObject("DescriptorSets", descriptorSet);
    }
}

void BindDescriptorSets::write(Output& output) const
{
    Object::write(output);

    output.writeObject("PipelineLayout", _pipelineLayout.get());

    output.write("firstSet", _firstSet);

    output.writeValue<uint32_t>("NumDescriptorSets", _descriptorSets.size());
    for (auto& descriptorSet : _descriptorSets)
    {
        output.writeObject("DescriptorSets", descriptorSet.get());
    }
}

void BindDescriptorSets::compile(Context& context)
{
    auto& vkd = _vulkanData[context.deviceID];

    // no need to compile if already compiled
    if (vkd._vkPipelineLayout != 0 && vkd._vkDescriptorSets.size() == _descriptorSets.size()) return;

    _pipelineLayout->compile(context);
    vkd._vkPipelineLayout = _pipelineLayout->vk(context.deviceID);

    vkd._vkDescriptorSets.resize(_descriptorSets.size());
    for (size_t i = 0; i < _descriptorSets.size(); ++i)
    {
        _descriptorSets[i]->compile(context);
        vkd._vkDescriptorSets[i] = _descriptorSets[i]->vk(context.deviceID);
    }
}

void BindDescriptorSets::dispatch(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindDescriptorSets(commandBuffer, _bindPoint, vkd._vkPipelineLayout, _firstSet, static_cast<uint32_t>(vkd._vkDescriptorSets.size()), vkd._vkDescriptorSets.data(), 0, nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BindDescriptorSet
//
BindDescriptorSet::BindDescriptorSet() :
    _bindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
    _firstSet(0)
{
}

void BindDescriptorSet::read(Input& input)
{
    _vulkanData.clear();

    StateCommand::read(input);

    input.readObject("PipelineLayout", _pipelineLayout);

    input.read("firstSet", _firstSet);

    input.readObject("DescriptorSet", _descriptorSet);
}

void BindDescriptorSet::write(Output& output) const
{
    StateCommand::write(output);

    output.writeObject("PipelineLayout", _pipelineLayout.get());

    output.write("firstSet", _firstSet);

    output.writeObject("DescriptorSet", _descriptorSet.get());
}

void BindDescriptorSet::compile(Context& context)
{
    auto& vkd = _vulkanData[context.deviceID];

    // no need to compile if already compiled
    if (vkd._vkPipelineLayout != 0 && vkd._vkDescriptorSet != 0) return;

    _pipelineLayout->compile(context);
    _descriptorSet->compile(context);

    vkd._vkPipelineLayout = _pipelineLayout->vk(context.deviceID);
    vkd._vkDescriptorSet = _descriptorSet->vk(context.deviceID);
}

void BindDescriptorSet::dispatch(CommandBuffer& commandBuffer) const
{
    auto& vkd = _vulkanData[commandBuffer.deviceID];

    vkCmdBindDescriptorSets(commandBuffer, _bindPoint, vkd._vkPipelineLayout, _firstSet, 1, &(vkd._vkDescriptorSet), 0, nullptr);
}
