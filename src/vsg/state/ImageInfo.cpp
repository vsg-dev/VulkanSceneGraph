/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/state/ImageInfo.h>

#include <algorithm>

using namespace vsg;

uint32_t vsg::computeNumMipMapLevels(const Data* data, const Sampler* sampler)
{
    uint32_t mipLevels = sampler != nullptr ? static_cast<uint32_t>(ceil(sampler->maxLod)) : 1;
    if (mipLevels == 0)
    {
        mipLevels = 1;
    }

    // clamp the mipLevels so that its no larger than what the data dimensions support
    uint32_t maxDimension = std::max({data->width(), data->height(), data->depth()});
    while ((1u << (mipLevels - 1)) > maxDimension)
    {
        --mipLevels;
    }

    //mipLevels = 1;  // disable mipmapping

    return mipLevels;
}

ImageInfo::~ImageInfo()
{
}

void ImageInfo::computeNumMipMapLevels()
{
    if (imageView && imageView->image && imageView->image->data)
    {
        auto image = imageView->image;
        auto data = image->data;
        auto mipLevels = vsg::computeNumMipMapLevels(data, sampler);

        const auto& mipmapOffsets = image->data->computeMipmapOffsets();
        bool generatMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);

        if (generatMipmaps)
        {
            // check that the data isn't compressed.
            auto layout = data->getLayout();
            if (layout.blockWidth > 1 || layout.blockHeight > 1 || layout.blockDepth > 1)
            {
                sampler->maxLod = 0.0f;
                mipLevels = 1;
            }
        }

        image->mipLevels = mipLevels;
        imageView->subresourceRange.levelCount = mipLevels;

        if (generatMipmaps) image->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
}
