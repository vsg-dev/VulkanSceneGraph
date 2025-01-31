/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/vk/Context.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorBuffer
//
DescriptorBuffer::DescriptorBuffer() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
{
}

DescriptorBuffer::DescriptorBuffer(const DescriptorBuffer& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    bufferInfoList(copyop(rhs.bufferInfoList))
{
}

DescriptorBuffer::DescriptorBuffer(ref_ptr<Data> data, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    if (data)
    {
        bufferInfoList.emplace_back(BufferInfo::create(data));
    }
}

DescriptorBuffer::DescriptorBuffer(const DataList& dataList, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    bufferInfoList.reserve(dataList.size());
    for (auto& data : dataList)
    {
        bufferInfoList.emplace_back(BufferInfo::create(data));
    }
}

DescriptorBuffer::DescriptorBuffer(const BufferInfoList& in_bufferInfoList, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType),
    bufferInfoList(in_bufferInfoList)
{
}

DescriptorBuffer::~DescriptorBuffer()
{
}

int DescriptorBuffer::compare(const Object& rhs_object) const
{
    int result = Descriptor::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    return compare_pointer_container(bufferInfoList, rhs.bufferInfoList);
}

void DescriptorBuffer::read(Input& input)
{
    Descriptor::read(input);

    bufferInfoList.clear();

    bufferInfoList.resize(input.readValue<uint32_t>("dataList"));
    for (auto& bufferInfo : bufferInfoList)
    {
        bufferInfo = vsg::BufferInfo::create();
        bufferInfo->buffer = nullptr;
        bufferInfo->offset = 0;
        bufferInfo->range = 0;
        input.readObject("data", bufferInfo->data);
    }
}

void DescriptorBuffer::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("dataList", bufferInfoList.size());
    for (const auto& bufferInfo : bufferInfoList)
    {
        output.writeObject("data", bufferInfo->data.get());
    }
}

void DescriptorBuffer::compile(Context& context)
{
    if (bufferInfoList.empty()) return;

    auto transferTask = context.transferTask.get();

    VkBufferUsageFlags bufferUsageFlags = 0;
    switch (descriptorType)
    {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    default:
        break;
    }

    bool requiresAssignmentOfBuffers = false;
    for (const auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->buffer == nullptr) requiresAssignmentOfBuffers = true;
    }

    auto deviceID = context.deviceID;

    if (requiresAssignmentOfBuffers)
    {
        VkDeviceSize alignment = 4;
        const auto& limits = context.device->getPhysicalDevice()->getProperties().limits;
        if (bufferUsageFlags == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            alignment = limits.minUniformBufferOffsetAlignment;
        else if (bufferUsageFlags == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            alignment = limits.minStorageBufferOffsetAlignment;

        VkDeviceSize totalSize = 0;

        // compute the total size of BufferInfo that needs to be allocated.
        {
            VkDeviceSize offset = 0;
            for (const auto& bufferInfo : bufferInfoList)
            {
                if (bufferInfo->data && !bufferInfo->buffer)
                {
                    totalSize = offset + bufferInfo->data->dataSize();
                    offset = (alignment == 1 || (totalSize % alignment) == 0) ? totalSize : ((totalSize / alignment) + 1) * alignment;
                    if (bufferInfo->data->dynamic() || transferTask) bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                }
            }
        }

        // if required allocate the buffer and reserve slots in it for the BufferInfo
        if (totalSize > 0)
        {
            auto buffer = vsg::Buffer::create(totalSize, bufferUsageFlags, VK_SHARING_MODE_EXCLUSIVE);
            for (const auto& bufferInfo : bufferInfoList)
            {
                if (bufferInfo->data && !bufferInfo->buffer)
                {
                    auto [allocated, offset] = buffer->reserve(bufferInfo->data->dataSize(), alignment);
                    if (allocated)
                    {
                        bufferInfo->buffer = buffer;
                        bufferInfo->offset = offset;
                        bufferInfo->range = bufferInfo->data->dataSize();
                    }
                    else
                    {
                        warn("DescriptorBuffer::compile(..) unable to allocate bufferInfo within associated Buffer.");
                    }
                }
            }
        }
    }

    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->buffer)
        {
            if (bufferInfo->buffer->compile(context.device))
            {
                if (bufferInfo->buffer->getDeviceMemory(deviceID) == nullptr)
                {
                    auto memRequirements = bufferInfo->buffer->getMemoryRequirements(deviceID);
                    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                    auto [deviceMemory, offset] = context.deviceMemoryBufferPools->reserveMemory(memRequirements, flags);
                    if (deviceMemory)
                    {
                        bufferInfo->buffer->bind(deviceMemory, offset);
                    }
                    else
                    {
                        throw Exception{"Error: DescriptorBuffer::compile(..) failed to allocate buffer from deviceMemoryBufferPools.", VK_ERROR_OUT_OF_DEVICE_MEMORY};
                    }
                }
            }

            if (!transferTask && bufferInfo->data && bufferInfo->data->getModifiedCount(bufferInfo->copiedModifiedCounts[deviceID]))
            {
                bufferInfo->copyDataToBuffer(context.deviceID);
            }
        }
    }

    if (transferTask) transferTask->assign(bufferInfoList);
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
        info.buffer = data->buffer->vk(context.deviceID);
        info.offset = data->offset;
        info.range = data->range;
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
        bufferInfo->copyDataToBuffer();
    }
}
