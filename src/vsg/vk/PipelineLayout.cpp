/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/PipelineLayout.h>

using namespace vsg;

//////////////////////////////////////
//
// PipelineLayout
//
PipelineLayout::PipelineLayout() :
    _flags(0)
{
}

PipelineLayout::PipelineLayout(const DescriptorSetLayouts& descriptorSetLayouts, const PushConstantRanges& pushConstantRanges, VkPipelineLayoutCreateFlags flags) :
    _descriptorSetLayouts(descriptorSetLayouts),
    _pushConstantRanges(pushConstantRanges),
    _flags(flags)
{
}

PipelineLayout::~PipelineLayout()
{
}

void PipelineLayout::read(Input& input)
{
    Object::read(input);

    _flags = static_cast<VkPipelineLayoutCreateFlags>(input.readValue<uint32_t>("Flags"));

    _descriptorSetLayouts.resize(input.readValue<uint32_t>("NumDescriptorSetLayouts"));
    for (auto& descriptorLayout : _descriptorSetLayouts)
    {
        descriptorLayout = input.readObject<DescriptorSetLayout>("DescriptorSetLayout");
    }

    _pushConstantRanges.resize(input.readValue<uint32_t>("NumPushConstantRanges"));
    for (auto& pushConstantRange : _pushConstantRanges)
    {
        pushConstantRange.stageFlags = static_cast<VkShaderStageFlags>(input.readValue<uint32_t>("stageFlags"));
        input.read("offset", pushConstantRange.offset);
        input.read("size", pushConstantRange.size);
    }
}

void PipelineLayout::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("Flags", _flags);

    output.writeValue<uint32_t>("NumDescriptorSetLayouts", _descriptorSetLayouts.size());
    for (auto& descriptorLayout : _descriptorSetLayouts)
    {
        output.writeObject("DescriptorSetLayout", descriptorLayout);
    }

    output.writeValue<uint32_t>("NumPushConstantRanges", _pushConstantRanges.size());
    for (auto& pushConstantRange : _pushConstantRanges)
    {
        output.writeValue<uint32_t>("stageFlags", pushConstantRange.stageFlags);
        output.write("offset", pushConstantRange.offset);
        output.write("size", pushConstantRange.size);
    }
}

void PipelineLayout::compile(Context& context)
{
    if (!_implementation)
    {
        for (auto dsl : _descriptorSetLayouts)
        {
            dsl->compile(context);
        }
        _implementation = PipelineLayout::Implementation::create(context.device, _descriptorSetLayouts, _pushConstantRanges, _flags);
    }
}

//////////////////////////////////////
//
// PipelineLayout::Implementation
//
PipelineLayout::Implementation::Implementation(VkPipelineLayout pipelineLayout, const DescriptorSetLayouts& descriptorSetLayouts, Device* device, AllocationCallbacks* allocator) :
    _pipelineLayout(pipelineLayout),
    _descriptorSetLayouts(descriptorSetLayouts),
    _device(device),
    _allocator(allocator)
{
}

PipelineLayout::Implementation::~Implementation()
{
    if (_pipelineLayout)
    {
        vkDestroyPipelineLayout(*_device, _pipelineLayout, _allocator);
    }
}

PipelineLayout::Implementation::Result PipelineLayout::Implementation::create(Device* device, const DescriptorSetLayouts& descriptorSetLayouts, const PushConstantRanges& pushConstantRanges, VkPipelineLayoutCreateFlags flags, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::PipelineLayout::create(...) failed to create PipelineLayout, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    std::vector<VkDescriptorSetLayout> layouts;
    for (auto& dsl : descriptorSetLayouts)
    {
        layouts.push_back(*dsl);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.flags = flags;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    pipelineLayoutInfo.pNext = nullptr;

    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(*device, &pipelineLayoutInfo, allocator, &pipelineLayout);
    if (result == VK_SUCCESS)
    {
        return Result(new PipelineLayout::Implementation(pipelineLayout, descriptorSetLayouts, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create PipelineLayout.", result);
    }
}
