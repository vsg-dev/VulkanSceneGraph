#pragma once

#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Buffer.h>

#include <cstring>

namespace vsg
{
    class BufferData
    {
    public:
        BufferData(Buffer* buffer, VkDeviceSize offset, VkDeviceSize range, Data* data=nullptr):
            _buffer(buffer),
            _offset(offset),
            _range(range),
            _data(data) {}

        ref_ptr<Buffer> _buffer;
        VkDeviceSize    _offset;
        VkDeviceSize    _range;
        ref_ptr<Data>   _data;
    };

    using BufferDataList = std::vector<BufferData>;

    typedef std::vector<ref_ptr<Data>> DataList;

    BufferDataList createBufferAndTransferData(Device* device, CommandPool* commandPool, VkQueue graphicsQueue, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    BufferDataList createHostVisibleBuffer(Device* device, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    void copyDataListToBuffers(BufferDataList& bufferDataList);

}
