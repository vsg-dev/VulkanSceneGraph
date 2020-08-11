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

    class VSG_DECLSPEC ImageView : public Inherit<Object, ImageView>
    {
    public:

        struct CreateInfo : public Inherit<Object, CreateInfo>
        {
            CreateInfo(ref_ptr<Image> in_image);

            VkImageViewCreateFlags     flags = 0;
            ref_ptr<Image>             image;
            VkImageViewType            viewType = VK_IMAGE_VIEW_TYPE_3D;
            VkFormat                   format = VK_FORMAT_UNDEFINED;
            VkComponentMapping         components;
            VkImageSubresourceRange    subresourceRange;

            virtual void apply(Context& context, VkImageViewCreateInfo& info);
        };

        ImageView(ref_ptr<CreateInfo> in_createInfo);
        ImageView(Device* device, const VkImageViewCreateInfo& createInfo);
        ImageView(Device* device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags);
        ImageView(Device* device, Image* image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags);

        ref_ptr<CreateInfo> createInfo;

        VkImageView vk(uint32_t deviceID) const { return _vulkanData[deviceID].imageView; }

        void setImage(Image* image) { _image = image; }
        Image* getImage() { return _image; }
        const Image* getImage() const { return _image; }

        void compile(Context& context);

    protected:
        virtual ~ImageView();

        struct VulkanData
        {
            VkImageView imageView = VK_NULL_HANDLE;
            ref_ptr<Device> device;

            ~VulkanData() { release(); }
            void release();
        };

        ref_ptr<Image> _image;
        vk_buffer<VulkanData> _vulkanData;

    };
    VSG_type_name(vsg::ImageView);

    using ImageViews = std::vector<ref_ptr<ImageView>>;

    /// convinience function that create an ImageView and allocates device memory and an Image for it. For device memory allocattion the Context's DeviceMemoryPools are utilized.
    extern VSG_DECLSPEC ref_ptr<ImageView> createImageView(Context& context, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags);

    /// convinience function that create an ImageView and allocates device memory and an Image for it.
    extern VSG_DECLSPEC ref_ptr<ImageView> createImageView(Device* device, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags);

} // namespace vsg
