#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/Sampler.h>
#include <vsg/vk/BufferData.h>
#include <vsg/state/ImageView.h>

namespace vsg
{
    class ImageData
    {
    public:
        ImageData() :
            imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {}

        ImageData(const ImageData& id) :
            sampler(id.sampler),
            imageView(id.imageView),
            imageLayout(id.imageLayout) {}

        ImageData(Sampler* in_sampler, ImageView* in_imageView, VkImageLayout in_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED) :
            sampler(in_sampler),
            imageView(in_imageView),
            imageLayout(in_imageLayout) {}

        ImageData& operator=(const ImageData& rhs)
        {
            sampler = rhs.sampler;
            imageView = rhs.imageView;
            imageLayout = rhs.imageLayout;
            return *this;
        }

        explicit operator bool() const { return sampler.valid() && imageView.valid(); }

        ref_ptr<Sampler> sampler;
        ref_ptr<ImageView> imageView;
        VkImageLayout imageLayout;
    };

    extern VSG_DECLSPEC BufferData copyDataToStagingBuffer(Context& context, const Data* data);
    extern VSG_DECLSPEC uint32_t computeNumMipMapLevels(const Data* data, const Sampler* sampler);
    extern VSG_DECLSPEC ImageData createImageData(Context& context, const Data* data, Sampler* sampler, VkImageLayout targetImageLayout, uint32_t mipLevels);

    /// transfer Data to graphics memory, returning ImageData configuration.
    extern VSG_DECLSPEC ImageData transferImageData(Context& context, const Data* data, Sampler* sampler = nullptr, VkImageLayout targetImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    using ImageDataList = std::vector<ImageData>;

} // namespace vsg
