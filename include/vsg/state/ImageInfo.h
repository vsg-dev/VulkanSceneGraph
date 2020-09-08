#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/Sampler.h>
#include <vsg/state/ImageView.h>

namespace vsg
{
    /// Settings that map to VkDescriptorImageInfo
    class VSG_DECLSPEC ImageInfo
    {
    public:
        ImageInfo() :
            imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {}

        ImageInfo(const ImageInfo& id) :
            sampler(id.sampler),
            imageView(id.imageView),
            imageLayout(id.imageLayout) {}

        ImageInfo(Sampler* in_sampler, ImageView* in_imageView, VkImageLayout in_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED) :
            sampler(in_sampler),
            imageView(in_imageView),
            imageLayout(in_imageLayout) {}

        ImageInfo& operator=(const ImageInfo& rhs)
        {
            sampler = rhs.sampler;
            imageView = rhs.imageView;
            imageLayout = rhs.imageLayout;
            return *this;
        }

        explicit operator bool() const { return sampler.valid() && imageView.valid(); }

        void computeNumMipMapLevels();

        ref_ptr<Sampler> sampler;
        ref_ptr<ImageView> imageView;
        VkImageLayout imageLayout;
    };
    using ImageInfoList = std::vector<ImageInfo>;

    // depcreated name for class
    using ImageData = ImageInfo;

} // namespace vsg
