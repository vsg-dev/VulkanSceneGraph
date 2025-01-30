/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/vk/Context.h>

using namespace vsg;

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::DescriptorSet(const DescriptorSet& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    setLayout(copyop(rhs.setLayout)),
    descriptors(copyop(rhs.descriptors))
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

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer(setLayout, rhs.setLayout))) return result;
    return compare_pointer_container(descriptors, rhs.descriptors);
}

void DescriptorSet::read(Input& input)
{
    Object::read(input);

    input.readObject("setLayout", setLayout);
    input.readObjects("descriptors", descriptors);
}

void DescriptorSet::write(Output& output) const
{
    Object::write(output);

    output.writeObject("setLayout", setLayout);
    output.writeObjects("descriptors", descriptors);
}

void DescriptorSet::compile(Context& context)
{
    if (!_implementation[context.deviceID])
    {
        // make sure all the contributing objects are compiled
        if (setLayout) setLayout->compile(context);
        for (auto& descriptor : descriptors) descriptor->compile(context);

        _implementation[context.deviceID] = context.allocateDescriptorSet(setLayout);
        _implementation[context.deviceID]->assign(context, descriptors);
    }
}

void DescriptorSet::release(uint32_t deviceID)
{
    Implementation::recycle(_implementation[deviceID]);
}
void DescriptorSet::release()
{
    for (auto& dsi : _implementation) Implementation::recycle(dsi);
    _implementation.clear();
}

VkDescriptorSet DescriptorSet::vk(uint32_t deviceID) const
{
    return _implementation[deviceID]->_descriptorSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorSet::Implementation
//
DescriptorSet::Implementation::Implementation(DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout) :
    _descriptorPool(descriptorPool),
    _descriptorSetLayout(descriptorSetLayout)
{
    auto device = descriptorPool->getDevice();

    VkDescriptorSetLayout vkdescriptorSetLayout = descriptorSetLayout->vk(device->deviceID);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &vkdescriptorSetLayout;

    // no need to locally lock DescriptorPool as the DescriptorSet::Implementation constructor should only be called by
    // DescriptorPool::allocateDescriptorSet that will already have locked the DescriptorPool::mutex before calling this constructor.
    // otherwise we'd need a : std::scoped_lock<std::mutex> lock(_descriptorPool->mutex);

    if (VkResult result = vkAllocateDescriptorSets(*device, &descriptorSetAllocateInfo, &_descriptorSet); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create DescriptorSet.", result};
    }
}

DescriptorSet::Implementation::~Implementation()
{
    if (_descriptorPool && _descriptorSet)
    {
        std::scoped_lock<std::mutex> lock(_descriptorPool->mutex);
        vkFreeDescriptorSets(*(_descriptorPool->getDevice()), *_descriptorPool, 1, &_descriptorSet);
    }
}

void DescriptorSet::Implementation::assign(Context& context, const Descriptors& in_descriptors)
{
    if (in_descriptors.empty()) return;

    VkWriteDescriptorSet* descriptorWrites = context.scratchMemory->allocate<VkWriteDescriptorSet>(in_descriptors.size());

    for (size_t i = 0; i < in_descriptors.size(); ++i)
    {
        in_descriptors[i]->assignTo(context, descriptorWrites[i]);
        descriptorWrites[i].dstSet = _descriptorSet;
    }

    auto device = _descriptorPool->getDevice();
    vkUpdateDescriptorSets(*device, static_cast<uint32_t>(in_descriptors.size()), descriptorWrites, 0, nullptr);

    // clean up scratch memory so it can be reused.
    context.scratchMemory->release();
}

void DescriptorSet::Implementation::recycle(ref_ptr<DescriptorSet::Implementation>& dsi)
{
    if (dsi)
    {
        if (dsi->_descriptorPool) dsi->_descriptorPool->freeDescriptorSet(dsi);
        dsi = {};
    }
}
