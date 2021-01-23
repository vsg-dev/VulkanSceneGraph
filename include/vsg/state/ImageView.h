#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/Image.h>

namespace vsg
{
    class Context;

    extern VSG_DECLSPEC VkImageAspectFlags computeAspectFlagsForFormat(VkFormat format);

    class VSG_DECLSPEC ImageView : public Inherit<Object, ImageView>
    {
    public:
        ImageView(ref_ptr<Image> in_image = {});
        ImageView(ref_ptr<Image> in_image, VkImageAspectFlags aspectFlags);

        /// VkImageViewCreateInfo settings
        VkImageViewCreateFlags flags = 0;
        ref_ptr<Image> image;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkComponentMapping components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        VkImageSubresourceRange subresourceRange;

        /// Vulkan VkImageView handle
        VkImageView vk(uint32_t deviceID) const { return _vulkanData[deviceID].imageView; }

        virtual void compile(Device* device);
        virtual void compile(Context& context);

    protected:
        virtual ~ImageView();

        struct VulkanData
        {
            VkImageView imageView = VK_NULL_HANDLE;
            ref_ptr<Device> device;

            ~VulkanData() { release(); }
            void release();
        };

        vk_buffer<VulkanData> _vulkanData;
    };
    VSG_type_name(vsg::ImageView);

    using ImageViews = std::vector<ref_ptr<ImageView>>;

    /// convenience function that create an ImageView and allocates device memory and an Image for it. For device memory allocation the Context's DeviceMemoryPools are utilized.
    extern VSG_DECLSPEC ref_ptr<ImageView> createImageView(Context& context, ref_ptr<Image> image, VkImageAspectFlags aspectFlags);

    /// convenience function that create an ImageView and allocates device memory and an Image for it.
    extern VSG_DECLSPEC ref_ptr<ImageView> createImageView(Device* device, ref_ptr<Image> image, VkImageAspectFlags aspectFlags);

} // namespace vsg
