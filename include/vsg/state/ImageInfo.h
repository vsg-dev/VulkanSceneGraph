#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/ImageView.h>
#include <vsg/state/Sampler.h>

namespace vsg
{
    extern VSG_DECLSPEC uint32_t computeNumMipMapLevels(const Data* data, const Sampler* sampler);

    /// Settings that map to VkDescriptorImageInfo
    class VSG_DECLSPEC ImageInfo : public Inherit<Object, ImageInfo>
    {
    public:
        ImageInfo() :
            imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {}

        ImageInfo(ref_ptr<Sampler> in_sampler, ref_ptr<ImageView> in_imageView, VkImageLayout in_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED) :
            sampler(in_sampler),
            imageView(in_imageView),
            imageLayout(in_imageLayout) {}

        // Convenience constructor that creates a vsg::ImageView and vsg::Image to represent the data on the GPU.
        template<typename T>
        ImageInfo(ref_ptr<Sampler> in_sampler, ref_ptr<T> in_data, VkImageLayout in_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) :
            sampler(in_sampler),
            imageLayout(in_imageLayout)
        {
            auto image = Image::create(in_data);
            image->usage |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

            imageView = ImageView::create(image);
        }

        ImageInfo(const ImageInfo&) = delete;
        ImageInfo& operator=(const ImageInfo&) = delete;

        explicit operator bool() const { return sampler.valid() && imageView.valid(); }

        void computeNumMipMapLevels();

        ref_ptr<Sampler> sampler;
        ref_ptr<ImageView> imageView;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    protected:
        virtual ~ImageInfo();
    };
    VSG_type_name(vsg::ImageInfo);

    using ImageInfoList = std::vector<ref_ptr<ImageInfo>>;

} // namespace vsg
