/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/state/ImageInfo.h>

#include <algorithm>

using namespace vsg;

FormatTraits vsg::getFormatTraits(VkFormat format, bool default_one)
{
    FormatTraits traits;

    if (VK_FORMAT_R8_UNORM <= format && format <= VK_FORMAT_B8G8R8A8_SRGB)
    {
        traits.numBitsPerComponent = 8;

        if (format <= VK_FORMAT_R8_SRGB)
            traits.numComponents = 1;
        else if (format <= VK_FORMAT_R8G8_SRGB)
            traits.numComponents = 2;
        else if (format <= VK_FORMAT_B8G8R8_SRGB)
            traits.numComponents = 3;
        else
            traits.numComponents = 4;

        switch ((format - VK_FORMAT_R8_UNORM) % 7)
        {
        case 0:
        case 2:
        case 4:
        case 6: traits.assign4<uint8_t>(default_one ? std::numeric_limits<uint8_t>::max() : 0); break;
        default: traits.assign4<int8_t>(default_one ? std::numeric_limits<int8_t>::max() : 0); break;
        }

        traits.size = traits.numComponents;
    }
    else if (VK_FORMAT_R16_UNORM <= format && format <= VK_FORMAT_R16G16B16A16_SFLOAT)
    {
        traits.numBitsPerComponent = 16;
        traits.numComponents = 1 + (format - VK_FORMAT_R16_UNORM) / 7;
        traits.size = 2 * traits.numComponents;

        switch ((format - VK_FORMAT_R16_UNORM) % 7)
        {
        case 0:
        case 2:
        case 4:
        case 6: traits.assign4<uint16_t>(default_one ? std::numeric_limits<uint16_t>::max() : 0); break;
        default: traits.assign4<int16_t>(default_one ? std::numeric_limits<int16_t>::max() : 0); break;
        }
    }
    else if (VK_FORMAT_R32_UINT <= format && format <= VK_FORMAT_R32G32B32A32_SFLOAT)
    {
        traits.numBitsPerComponent = 32;
        traits.numComponents = 1 + (format - VK_FORMAT_R32_UINT) / 3;
        traits.size = 4 * traits.numComponents;

        switch ((format - VK_FORMAT_R32_UINT) % 3)
        {
        case 0: traits.assign4<uint32_t>(default_one ? std::numeric_limits<uint32_t>::max() : 0); break;
        case 1: traits.assign4<int32_t>(default_one ? std::numeric_limits<int32_t>::max() : 0); break;
        case 2: traits.assign4<float>(default_one ? 1.0f : 0.0f); break;
        }
    }
    else if (VK_FORMAT_R64_UINT <= format && format <= VK_FORMAT_R64G64B64A64_SFLOAT)
    {
        traits.numBitsPerComponent = 64;
        traits.numComponents = 1 + (format - VK_FORMAT_R64_UINT) / 3;
        traits.size = 8 * traits.numComponents;

        switch ((format - VK_FORMAT_R64_UINT) % 3)
        {
        case 0: traits.assign4<uint64_t>(default_one ? std::numeric_limits<uint64_t>::max() : 0); break;
        case 1: traits.assign4<int64_t>(default_one ? std::numeric_limits<int64_t>::max() : 0); break;
        case 2: traits.assign4<double>(default_one ? 1.0 : 0.0); break;
        }
    }

    return traits;
}

uint32_t vsg::computeNumMipMapLevels(const Data* data, const Sampler* sampler)
{
    return computeNumMipMapLevels(data->width(), data->height(), data->depth(), sampler);
}

uint32_t vsg::computeNumMipMapLevels(uint32_t w, uint32_t h, uint32_t d, const Sampler* sampler)
{
    uint32_t mipLevels = 1;
    if (sampler)
    {
        mipLevels = vsg::computeNumMipMapLevels(w, h, d);
        if (sampler->maxLod != VK_LOD_CLAMP_NONE)
        {
            mipLevels = std::max(1u, std::min(static_cast<uint32_t>(sampler->maxLod), mipLevels));
        }
    }

    //mipLevels = 1;  // disable mipmapping

    return mipLevels;
}

ImageInfo::ImageInfo(ref_ptr<Sampler> in_sampler, ref_ptr<ImageView> in_imageView, VkImageLayout in_imageLayout) :
    sampler(in_sampler),
    imageView(in_imageView),
    imageLayout(in_imageLayout)
{
}

ImageInfo::~ImageInfo()
{
}

int ImageInfo::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer(sampler, rhs.sampler))) return result;
    if ((result = compare_pointer(imageView, rhs.imageView))) return result;
    return compare_value(imageLayout, rhs.imageLayout);
}

void ImageInfo::computeNumMipMapLevels()
{
    if (imageView && imageView->image && imageView->image->data)
    {
        auto image = imageView->image;
        auto data = image->data;
        uint32_t mipLevels = vsg::computeNumMipMapLevels(image->extent.width / data->properties.blockWidth, image->extent.height / data->properties.blockHeight, image->extent.depth / data->properties.blockDepth, sampler);

        bool generateMipmaps = false;
        if (data->properties.maxNumMipmaps <= 1 && mipLevels > 1)
        {
            generateMipmaps = true;
        }
        else
        {
            mipLevels = vsg::computeNumMipMapLevels(data->properties, image);
        }

        if (generateMipmaps)
        {
            // check that the data isn't compressed.
            const auto& properties = data->properties;
            if (properties.blockWidth > 1 || properties.blockHeight > 1 || properties.blockDepth > 1)
            {
                if (sampler->maxLod != 0.0f && sampler->maxLod != VK_LOD_CLAMP_NONE)
                {
                    warn("ImageInfo::computeNumMipMapLevels() cannot enable generated mipmaps for vsg::Image, but Sampler::maxLod is not zero or VK_LOD_CLAMP_NONE, sampler->maxLod = ", sampler->maxLod);
                }

                mipLevels = 1;
                generateMipmaps = false;
            }
        }

        image->mipLevels = mipLevels;

        if (generateMipmaps) image->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
}

uint32_t vsg::computeNumMipMapLevels(uint32_t w, uint32_t h, uint32_t d)
{
    // clamp the mipLevels so that it's no larger than what the data dimensions support
    uint32_t maxDimension = std::max({w, h, d});
    uint32_t mipLevels = 1;
    while ((1u << mipLevels) <= maxDimension)
    {
        ++mipLevels;
    }
    return mipLevels;
}

uint32_t vsg::computeNumMipMapLevels(const vsg::Data::Properties& properties, const Image* image)
{
    // get the dimensions, usually equivalent to Data::width/height/depth() except for layered images
    uint32_t w = image->extent.width / properties.blockWidth;
    uint32_t h = image->extent.height / properties.blockHeight;
    uint32_t d = image->extent.depth / properties.blockDepth;
    // clamp the mipLevels so that it's no larger than what the data dimensions support
    uint32_t mipLevels = vsg::computeNumMipMapLevels(w, h, d);

    // clamp the mipLevels so that it's no larger than specified by vsg::Data
    mipLevels = std::min(std::max(1u, static_cast<uint32_t>(properties.maxNumMipmaps)), mipLevels);

    // clamp the mipLevels so that it's no larger than the mipLevels vsg::Image was compiled with
    if (image->mipLevels > 0)
        mipLevels = std::min(image->mipLevels, mipLevels);
    return mipLevels;
}

size_t vsg::computeValueCount(const vsg::Data::Properties& properties, const vsg::Image* image)
{
    uint32_t w = image->extent.width / properties.blockWidth;
    uint32_t h = image->extent.height / properties.blockHeight;
    uint32_t d = image->extent.depth / properties.blockDepth;
    uint32_t mipLevels = vsg::computeNumMipMapLevels(properties, image);
    return Data::computeValueCountIncludingMipmaps(w, h, d, mipLevels, image->arrayLayers);
}
