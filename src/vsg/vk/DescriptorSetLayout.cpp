/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/DescriptorSetLayout.h>

using namespace vsg;

//////////////////////////////////////
//
// DescriptorSetLayout
//
DescriptorSetLayout::DescriptorSetLayout()
{
}

DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutBindings& descriptorSetLayoutBindings) :
    _descriptorSetLayoutBindings(descriptorSetLayoutBindings)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

void DescriptorSetLayout::read(Input& input)
{
    Object::read(input);

    _descriptorSetLayoutBindings.resize(input.readValue<uint32_t>("NumDescriptorSetLayoutBindings"));
    for (auto& dslb : _descriptorSetLayoutBindings)
    {
        input.read("binding", dslb.binding);
        dslb.descriptorType = static_cast<VkDescriptorType>(input.readValue<uint32_t>("descriptorType"));
        input.read("descriptorCount", dslb.descriptorCount);
        dslb.stageFlags = input.readValue<uint32_t>("stageFlags");
    }
}

void DescriptorSetLayout::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("NumDescriptorSetLayoutBindings", _descriptorSetLayoutBindings.size());
    for (auto& dslb : _descriptorSetLayoutBindings)
    {
        output.write("binding", dslb.binding);
        output.writeValue<uint32_t>("descriptorType", dslb.descriptorType);
        output.write("descriptorCount", dslb.descriptorCount);
        output.writeValue<uint32_t>("stageFlags", dslb.stageFlags);
    }
}

void DescriptorSetLayout::compile(Context& context)
{
    if (!_implementation) _implementation = DescriptorSetLayout::Implementation::create(context.device, _descriptorSetLayoutBindings);
}

//////////////////////////////////////
//
// DescriptorSetLayout::Implementation
//
DescriptorSetLayout::Implementation::Implementation(Device* device, VkDescriptorSetLayout descriptorSetLayout, AllocationCallbacks* allocator) :
    _device(device),
    _descriptorSetLayout(descriptorSetLayout),
    _allocator(allocator)
{
}

DescriptorSetLayout::Implementation::~Implementation()
{
    if (_descriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(*_device, _descriptorSetLayout, _allocator);
    }
}

DescriptorSetLayout::Implementation::Result DescriptorSetLayout::Implementation::create(Device* device, const DescriptorSetLayoutBindings& descriptorSetLayoutBindings, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::DescriptorSetLayout::create(...) failed to create DescriptorSetLayout, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
    layoutInfo.pBindings = descriptorSetLayoutBindings.data();
    layoutInfo.pNext = nullptr;

    VkDescriptorSetLayout descriptorSetLayout;
    VkResult result = vkCreateDescriptorSetLayout(*device, &layoutInfo, allocator, &descriptorSetLayout);
    if (result == VK_SUCCESS)
    {
        return Result(new Implementation(device, descriptorSetLayout, allocator));
    }
    else
    {
        return Result("Error: Failed to create DescriptorSetLayout.", result);
    }
}
