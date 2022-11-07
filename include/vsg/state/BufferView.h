#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/Buffer.h>
#include <vsg/vk/vk_buffer.h>

namespace vsg
{
    // forward declare
    class Context;

    /// BufferView encapsulates VkBufferView and the VkBufferViewCreateInfo settings used to set it up.
    class VSG_DECLSPEC BufferView : public Inherit<Object, BufferView>
    {
    public:
        BufferView();
        BufferView(ref_ptr<Buffer> buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range);

        /// Vulkan VkImage handle
        VkBufferView vk(uint32_t deviceID) const { return _vulkanData[deviceID].bufferView; }

        // VkBufferViewCreateInfo settings
        ref_ptr<Buffer> buffer;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkDeviceSize offset = 0;
        VkDeviceSize range = 0;

        int compare(const Object& rhs_object) const override;

        virtual void compile(Device* device);
        virtual void compile(Context& context);

    protected:
        virtual ~BufferView();

        struct VulkanData
        {
            VkBufferView bufferView = VK_NULL_HANDLE;
            ref_ptr<Device> device;

            ~VulkanData() { release(); }
            void release();
        };

        vk_buffer<VulkanData> _vulkanData;
    };
    VSG_type_name(vsg::BufferView);

    using BufferViewList = std::vector<ref_ptr<BufferView>>;

} // namespace vsg
