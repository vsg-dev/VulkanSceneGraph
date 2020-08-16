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

        BufferData(Buffer* in_buffer, VkDeviceSize in_offset, VkDeviceSize in_range, Data* in_data = nullptr) :
            buffer(in_buffer),
            offset(in_offset),
            range(in_range),
            data(in_data) {}

        BufferData(const BufferData&) = default;

        BufferData& operator=(const BufferData&) = default;

        void release()
        {
            if (buffer)
            {
                buffer->release(offset, range);
            }

            buffer = 0;
            offset = 0;
            range = 0;
        }

        explicit operator bool() const { return buffer.valid() && data.valid() && range != 0; }

        ref_ptr<Buffer> buffer;
        VkDeviceSize offset = 0;
        VkDeviceSize range = 0;
        ref_ptr<Data> data;
    };

    using BufferDataList = std::vector<BufferData>;

    extern VSG_DECLSPEC BufferData copyDataToStagingBuffer(Context& context, const Data* data);

    extern VSG_DECLSPEC BufferDataList createBufferAndTransferData(Context& context, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    extern VSG_DECLSPEC BufferDataList createHostVisibleBuffer(Device* device, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    extern VSG_DECLSPEC void copyDataListToBuffers(BufferDataList& bufferDataList);

} // namespace vsg
