#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/ImageView.h>
#include <vsg/state/Sampler.h>

namespace vsg
{
    class ImageData
    {
    public:
        ImageData() :
            _imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {}

        ImageData(const ImageData& id) :
            _sampler(id._sampler),
            _imageView(id._imageView),
            _imageLayout(id._imageLayout) {}

        ImageData(Sampler* sampler, ImageView* imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED) :
            _sampler(sampler),
            _imageView(imageView),
            _imageLayout(imageLayout) {}

        ImageData& operator=(const ImageData& rhs)
        {
            _sampler = rhs._sampler;
            _imageView = rhs._imageView;
            _imageLayout = rhs._imageLayout;
            return *this;
        }

        explicit operator bool() const { return _sampler.valid() && _imageView.valid(); }

        bool valid() const { return _sampler.valid() && _imageView.valid(); }

        ref_ptr<Sampler> _sampler;
        ref_ptr<ImageView> _imageView;
        VkImageLayout _imageLayout;
    };

    /// transfer Data to graphics memory, returning ImageData configuration.
    extern VSG_DECLSPEC vsg::ImageData transferImageData(Context& context, const Data* data, Sampler* sampler = nullptr, VkImageLayout targetImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    using ImageDataList = std::vector<ImageData>;

} // namespace vsg
