#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Image.h>
#include <vsg/vk/vk_buffer.h>

namespace vsg
{
    class Context;


    class VSG_DECLSPEC ImageView : public Inherit<Object, ImageView>
    {
    public:
        ImageView(Device* device, const VkImageViewCreateInfo& createInfo);
        ImageView(Device* device, Image* image, VkImageViewType type, VkFormat in_format, VkImageAspectFlags aspectFlags);

        struct Settings
        {
            VkImageViewCreateFlags     flags;
            ref_ptr<Image>             image; // per context.
            VkImageViewType            viewType;
            VkFormat                   format;
            VkComponentMapping         components; // component swizzel for r g b a
            VkImageSubresourceRange    subresourceRange;
        };

#if 1
        void setImage(Image* in_image) { image = in_image; }
        Image* getImage() { return image; }
        const Image* getImage() const { return image; }
#endif

#if 0
         void compile(Context& context);
#endif

        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

        VkImageView vk(uint32_t deviceID) const { return _implementation[deviceID]->imageView; }
        operator VkImageView() const { return _implementation[0]->imageView; }

    protected:
        virtual ~ImageView();

        struct Implementation : public Object
        {
            Implementation(Device* in_device, const VkImageViewCreateInfo& createInfo);
            virtual ~Implementation();

            VkImageView imageView;
            ref_ptr<Device> device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
        ref_ptr<Image> image;
    };
    VSG_type_name(vsg::ImageView);

    using ImageViews = std::vector<ref_ptr<ImageView>>;

    /// convinience function that create an ImageView and allocates device memory and an Image for it. For device memory allocattion the Context's DeviceMemoryPools are utilized.
    extern VSG_DECLSPEC ref_ptr<ImageView> createImageView(Context& context, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags);

    /// convinience function that create an ImageView and allocates device memory and an Image for it.
    extern VSG_DECLSPEC ref_ptr<ImageView> createImageView(Device* device, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags);

} // namespace vsg
