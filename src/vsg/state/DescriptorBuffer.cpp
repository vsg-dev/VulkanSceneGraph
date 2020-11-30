/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>

#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorBuffer
//
DescriptorBuffer::DescriptorBuffer() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
{
}

DescriptorBuffer::DescriptorBuffer(ref_ptr<Data> data, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    if (data)
    {
        bufferInfoList.emplace_back(nullptr, 0, 0, data);
    }
}

DescriptorBuffer::DescriptorBuffer(const DataList& dataList, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    bufferInfoList.reserve(dataList.size());
    for (auto& data : dataList)
    {
        bufferInfoList.emplace_back(nullptr, 0, 0, data);
    }
}

DescriptorBuffer::DescriptorBuffer(const BufferInfoList& in_bufferInfoList, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType),
    bufferInfoList(in_bufferInfoList)
{
}

DescriptorBuffer::~DescriptorBuffer()
{
    for (auto& bufferInfo : bufferInfoList)
    {
        bufferInfo.release();
    }
}

void DescriptorBuffer::read(Input& input)
{
    for (auto& bufferInfo : bufferInfoList)
    {
        bufferInfo.release();
    }

    Descriptor::read(input);

    bufferInfoList.resize(input.readValue<uint32_t>("NumData"));
    for (auto& bufferInfo : bufferInfoList)
    {
        bufferInfo.buffer = nullptr;
        bufferInfo.offset = 0;
        bufferInfo.range = 0;
        input.readObject("Data", bufferInfo.data);
    }
}

void DescriptorBuffer::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumData", bufferInfoList.size());
    for (auto& bufferInfo : bufferInfoList)
    {
        output.writeObject("Data", bufferInfo.data.get());
    }
}

void DescriptorBuffer::compile(Context& context)
{
    VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    bool requiresAssingmentOfBuffers = false;
    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo.buffer == nullptr) requiresAssingmentOfBuffers = true;
    }

    if (requiresAssingmentOfBuffers)
    {
        VkDeviceSize alignment = 4;
        if (bufferUsageFlags == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) alignment = context.device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

        VkDeviceSize totalSize = 0;
        VkDeviceSize offset = 0;

        for (auto& bufferInfo : bufferInfoList)
        {
            if (bufferInfo.data)
            {
                bufferInfo.offset = offset;
                bufferInfo.range = bufferInfo.data->dataSize();

                totalSize = offset + bufferInfo.range;
                offset = (alignment == 1 || (totalSize % alignment) == 0) ? totalSize : ((totalSize / alignment) + 1) * alignment;
            }
        }

        auto buffer = vsg::Buffer::create(totalSize, bufferUsageFlags, VK_SHARING_MODE_EXCLUSIVE);

        for (auto& bufferInfo : bufferInfoList)
        {
            if (!bufferInfo.buffer) bufferInfo.buffer = buffer;
        }
    }

    bool needToCopyDataToBuffer = false;
    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo.buffer->compile(context.device))
        {
            if (bufferInfo.buffer->getDeviceMemory(context.deviceID) == nullptr)
            {
                auto memRequirements = bufferInfo.buffer->getMemoryRequirements(context.deviceID);
                auto memory = vsg::DeviceMemory::create(context.device, memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                bufferInfo.buffer->bind(memory, 0);
            }

            needToCopyDataToBuffer = true;
        }
    }

    if (needToCopyDataToBuffer)
    {
        for (auto& bufferInfo : bufferInfoList)
        {
            bufferInfo.copyDataToBuffer(context.deviceID);
        }
    }
}

void DescriptorBuffer::assignTo(Context& context, VkWriteDescriptorSet& wds) const
{
    Descriptor::assignTo(context, wds);

    auto pBufferInfo = context.scratchMemory->allocate<VkDescriptorBufferInfo>(bufferInfoList.size());
    wds.descriptorCount = static_cast<uint32_t>(bufferInfoList.size());
    wds.pBufferInfo = pBufferInfo;

    // convert from VSG to Vk
    for (size_t i = 0; i < bufferInfoList.size(); ++i)
    {
        auto& data = bufferInfoList[i];
        VkDescriptorBufferInfo& info = pBufferInfo[i];
        info.buffer = data.buffer->vk(context.deviceID);
        info.offset = data.offset;
        info.range = data.range;
    }
}

uint32_t DescriptorBuffer::getNumDescriptors() const
{
    return static_cast<uint32_t>(bufferInfoList.size());
}

void DescriptorBuffer::copyDataListToBuffers()
{
    for (auto& bufferInfo : bufferInfoList)
    {
        bufferInfo.copyDataToBuffer();
    }
}
