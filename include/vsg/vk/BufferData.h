#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Buffer.h>

#include <cstring>

namespace vsg
{
    // forward declare
    class Context;
    class CommandBuffer;

    class VSG_DECLSPEC BufferData
    {
    public:
        BufferData() = default;

        BufferData(Buffer* buffer, VkDeviceSize offset, VkDeviceSize range, Data* data = nullptr) :
            _buffer(buffer),
            _offset(offset),
            _range(range),
            _data(data) {}

        BufferData(const BufferData&) = default;

        BufferData& operator=(const BufferData&) = default;

        void release()
        {
            if (_buffer)
            {
                _buffer->release(_offset, _range);
            }

            _buffer = 0;
            _offset = 0;
            _range = 0;
        }

        ref_ptr<Buffer> _buffer;
        VkDeviceSize _offset = 0;
        VkDeviceSize _range = 0;
        ref_ptr<Data> _data;
    };

    using BufferDataList = std::vector<BufferData>;

    BufferDataList createBufferAndTransferData(Context& context, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    BufferDataList createHostVisibleBuffer(Device* device, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    void copyDataListToBuffers(BufferDataList& bufferDataList);

} // namespace vsg
