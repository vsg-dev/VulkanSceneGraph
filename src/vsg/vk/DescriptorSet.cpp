/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/State.h>

using namespace vsg;

DescriptorSet::DescriptorSet(const DescriptorSetLayouts& descriptorSetLayouts, const Descriptors& descriptors):
    _descriptorSetLayouts(descriptorSetLayouts),
    _descriptors(descriptors)
{
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::compile(Context& context)
{
    if (!_implementation)
    {
        // make sure all the contributing objects are compiled
        for(auto& descriptorSetLayout : _descriptorSetLayouts) descriptorSetLayout->compile(context);
        for(auto& descriptor : _descriptors) descriptor->compile(context);

        _implementation = DescriptorSet::Implementation::create(context.device, context.descriptorPool, _descriptorSetLayouts, _descriptors);
    }
}

DescriptorSet::Implementation::Implementation(VkDescriptorSet descriptorSet, Device* device, DescriptorPool* descriptorPool, const DescriptorSetLayouts& descriptorSetLayouts, const Descriptors& descriptors) :
    _descriptorSet(descriptorSet),
    _device(device),
    _descriptorPool(descriptorPool),
    _descriptorSetLayouts(descriptorSetLayouts)
{
    assign(descriptors);
}

DescriptorSet::Implementation::~Implementation()
{
    if (_descriptorSet)
    {
        vkFreeDescriptorSets(*_device, *_descriptorPool, 1, &_descriptorSet);
    }
}

DescriptorSet::Implementation::Result DescriptorSet::Implementation::create(Device* device, DescriptorPool* descriptorPool, const DescriptorSetLayouts& descriptorSetLayouts, const Descriptors& descriptors)
{
    if (!device || !descriptorPool || descriptorSetLayouts.empty())
    {
        return Result("Error: vsg::DescriptorPool::create(...) failed to create DescriptorPool, undefined Device, DescriptorPool or DescriptorSetLayouts.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    std::vector<VkDescriptorSetLayout> vkdescriptorSetLayouts;
    for(auto& descriptorSetLayout : descriptorSetLayouts)
    {
        vkdescriptorSetLayouts.push_back(*descriptorSetLayout);
    }


    VkDescriptorSetAllocateInfo descriptSetAllocateInfo = {};
    descriptSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(vkdescriptorSetLayouts.size());
    descriptSetAllocateInfo.pSetLayouts = vkdescriptorSetLayouts.data();

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(*device, &descriptSetAllocateInfo, &descriptorSet);
    if (result == VK_SUCCESS)
    {
        return Result(new DescriptorSet::Implementation(descriptorSet, device, descriptorPool, descriptorSetLayouts, descriptors));
    }
    else
    {
        return Result("Error: Failed to create DescriptorSet.", result);
    }
}

void DescriptorSet::Implementation::assign(const Descriptors& descriptors)
{
    // should we doing anything about previous _descriptor that may have been assigned?
    _descriptors = descriptors;

    std::vector<VkWriteDescriptorSet> descriptorWrites(_descriptors.size());
    for (size_t i = 0; i < _descriptors.size(); ++i)
    {
        _descriptors[i]->assignTo(descriptorWrites[i], _descriptorSet);
    }

    vkUpdateDescriptorSets(*_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void BindDescriptorSets::pushTo(State& state) const
{
    state.dirty = true;

    // make sure there is an appropriate descriptorStack entry available.
    if (_firstSet >= state.descriptorStacks.size())  state.descriptorStacks.resize(_firstSet+1);

    // push this to the appropriate descriptorStack
    state.descriptorStacks[_firstSet].push(this);
}

void BindDescriptorSets::popFrom(State& state) const
{
    state.dirty = true;
    state.descriptorStacks[_firstSet].pop();
}

void BindDescriptorSets::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindDescriptorSets(commandBuffer, _bindPoint, *(_pipelineLayout), _firstSet, static_cast<uint32_t>(_vkDescriptorSets.size()), _vkDescriptorSets.data(), 0, nullptr);
}

void BindDescriptorSets::compile(Context& context)
{
    _vkDescriptorSets.resize(_descriptorSets.size());
    for (size_t i = 0; i < _descriptorSets.size(); ++i)
    {
        _descriptorSets[i]->compile(context);

        _vkDescriptorSets[i] = *(_descriptorSets[i]);
    }
}
