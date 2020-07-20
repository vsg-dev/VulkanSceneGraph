/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/DescriptorBuffer.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/io/Options.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorBuffer
//
DescriptorBuffer::DescriptorBuffer() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
{
}

DescriptorBuffer::DescriptorBuffer(ref_ptr<Data> data, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (data) _dataList.emplace_back(data);
}

DescriptorBuffer::DescriptorBuffer(const DataList& dataList, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _dataList(dataList)
{
}

DescriptorBuffer::DescriptorBuffer(const BufferDataList& bufferDataList, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _bufferDataList(bufferDataList)
{
}

void DescriptorBuffer::read(Input& input)
{
    _bufferDataList.clear();

    Descriptor::read(input);

    _dataList.resize(input.readValue<uint32_t>("NumData"));
    for (auto& data : _dataList)
    {
        input.readObject("Data", data);
    }
}

void DescriptorBuffer::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumData", _dataList.size());
    for (auto& data : _dataList)
    {
        output.writeObject("Data", data.get());
    }
}

void DescriptorBuffer::compile(Context& context)
{
    // check if already compiled
    if (_bufferDataList.size() < _dataList.size())
    {
        VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
#if 1
        _bufferDataList = vsg::createHostVisibleBuffer(context.device, _dataList, bufferUsageFlags, VK_SHARING_MODE_EXCLUSIVE);
        vsg::copyDataListToBuffers(_bufferDataList);
#else
        _bufferDataList = vsg::createBufferAndTransferData(context, _dataList, bufferUsageFlags, VK_SHARING_MODE_EXCLUSIVE);
#endif
    }
}

void DescriptorBuffer::assignTo(Context& context, VkWriteDescriptorSet& wds) const
{
    Descriptor::assignTo(context, wds);

    auto pBufferInfo = context.scratchMemory->allocate<VkDescriptorBufferInfo>(_bufferDataList.size());
    wds.descriptorCount = static_cast<uint32_t>(_bufferDataList.size());
    wds.pBufferInfo = pBufferInfo;

    // convert from VSG to Vk
    for (size_t i = 0; i < _bufferDataList.size(); ++i)
    {
        const BufferData& data = _bufferDataList[i];
        VkDescriptorBufferInfo& info = pBufferInfo[i];
        info.buffer = *(data._buffer);
        info.offset = data._offset;
        info.range = data._range;
    }
}

uint32_t DescriptorBuffer::getNumDescriptors() const
{
    return static_cast<uint32_t>(std::max(_bufferDataList.size(), _dataList.size()));
}

void DescriptorBuffer::copyDataListToBuffers()
{
    vsg::copyDataListToBuffers(_bufferDataList);
}
