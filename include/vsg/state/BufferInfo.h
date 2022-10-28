#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/Buffer.h>

#include <cstring>

namespace vsg
{
    // forward declare
    class Context;
    class CommandBuffer;

    /// BufferInfo encapsulates the settings that map to VkDescriptorBufferInfo
    class VSG_DECLSPEC BufferInfo : public Inherit<Object, BufferInfo>
    {
    public:
        BufferInfo();
        explicit BufferInfo(Data* in_data);
        BufferInfo(Buffer* in_buffer, VkDeviceSize in_offset, VkDeviceSize in_range, Data* in_data = nullptr);

        BufferInfo(const BufferInfo&) = delete;
        BufferInfo& operator=(const BufferInfo&) = delete;

        int compare(const Object& rhs_object) const override;

        void release();

        /// Copy data to the VkBuffer(s) for all Devices associated with vsg::Buffer
        /// Requires associated buffer memory to be host visible, for non host visible buffers you must use a staging buffer
        void copyDataToBuffer();

        /// Copy data to the VkBuffer associated with the a specified Device
        /// Requires associated buffer memory to be host visible, for non host visible buffers you must use a staging buffer
        void copyDataToBuffer(uint32_t deviceID);

        explicit operator bool() const { return buffer.valid() && data.valid() && range != 0; }

        ref_ptr<Buffer> buffer;
        VkDeviceSize offset = 0;
        VkDeviceSize range = 0;
        ref_ptr<Data> data;
        ref_ptr<BufferInfo> parent;

        /// return true if the BufferInfo's data has been modified and should be copied to the buffer
        bool requiresCopy(uint32_t deviceID) const
        {
            return data && data->differentModifiedCount(copiedModifiedCounts[deviceID]);
        }

        /// return true if the BufferInfo's data has been modified and should be copied to the buffer, and sync the moificationCounts
        bool syncModifiedCounts(uint32_t deviceID)
        {
            return data && data->getModifiedCount(copiedModifiedCounts[deviceID]);
        }

        vk_buffer<ModifiedCount> copiedModifiedCounts;

    protected:
        virtual ~BufferInfo();
    };
    VSG_type_name(vsg::BufferInfo);

    using BufferInfoList = std::vector<ref_ptr<BufferInfo>>;

    struct VulkanArrayData
    {
        std::vector<VkBuffer> vkBuffers;
        std::vector<VkDeviceSize> offsets;
    };

    /// assign the Vulkan buffer handles and offsets held in BufferInfoList to VulkanArrayData
    extern VSG_DECLSPEC void assignVulkanArrayData(uint32_t deviceID, const BufferInfoList& arrays, VulkanArrayData& vkd);

    extern VSG_DECLSPEC ref_ptr<BufferInfo> copyDataToStagingBuffer(Context& context, const Data* data);

    extern VSG_DECLSPEC bool createBufferAndTransferData(Context& context, const BufferInfoList& bufferInfoList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    extern VSG_DECLSPEC BufferInfoList createHostVisibleBuffer(Device* device, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode);

    extern VSG_DECLSPEC void copyDataListToBuffers(Device* device, BufferInfoList& bufferInfoList);

} // namespace vsg
