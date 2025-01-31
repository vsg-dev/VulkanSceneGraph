/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/vk/Context.h>

using namespace vsg;

//////////////////////////////////////
//
// DescriptorSetLayout
//
DescriptorSetLayout::DescriptorSetLayout()
{
}

DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayout& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    bindings(rhs.bindings)
{
}

DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutBindings& descriptorSetLayoutBindings) :
    bindings(descriptorSetLayoutBindings)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

void DescriptorSetLayout::getDescriptorPoolSizes(DescriptorPoolSizes& descriptorPoolSizes)
{
    for (auto& binding : bindings)
    {
        auto itr = descriptorPoolSizes.begin();
        for (; itr != descriptorPoolSizes.end(); ++itr)
        {
            if (itr->type == binding.descriptorType)
            {
                itr->descriptorCount += binding.descriptorCount;
                break;
            }
        }
        if (itr == descriptorPoolSizes.end())
        {
            descriptorPoolSizes.emplace_back(VkDescriptorPoolSize{binding.descriptorType, binding.descriptorCount});
        }
    }
}

int DescriptorSetLayout::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value_container(bindings, rhs.bindings);
}

void DescriptorSetLayout::read(Input& input)
{
    Object::read(input);

    bindings.resize(input.readValue<uint32_t>("bindings"));

    for (auto& dslb : bindings)
    {
        input.read("binding", dslb.binding);
        input.readValue<uint32_t>("descriptorType", dslb.descriptorType);
        input.read("descriptorCount", dslb.descriptorCount);
        input.readValue<uint32_t>("stageFlags", dslb.stageFlags);
    }
}

void DescriptorSetLayout::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("bindings", bindings.size());

    for (auto& dslb : bindings)
    {
        output.write("binding", dslb.binding);
        output.writeValue<uint32_t>("descriptorType", dslb.descriptorType);
        output.write("descriptorCount", dslb.descriptorCount);
        output.writeValue<uint32_t>("stageFlags", dslb.stageFlags);
    }
}

void DescriptorSetLayout::compile(Context& context)
{
    if (!_implementation[context.deviceID]) _implementation[context.deviceID] = DescriptorSetLayout::Implementation::create(context.device, bindings);
}

//////////////////////////////////////
//
// DescriptorSetLayout::Implementation
//
DescriptorSetLayout::Implementation::Implementation(Device* device, const DescriptorSetLayoutBindings& descriptorSetLayoutBindings) :
    _device(device)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
    layoutInfo.pBindings = descriptorSetLayoutBindings.data();
    layoutInfo.pNext = nullptr;

    if (VkResult result = vkCreateDescriptorSetLayout(*device, &layoutInfo, _device->getAllocationCallbacks(), &_descriptorSetLayout); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create DescriptorSetLayout.", result};
    }
}

DescriptorSetLayout::Implementation::~Implementation()
{
    if (_descriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(*_device, _descriptorSetLayout, _device->getAllocationCallbacks());
    }
}
