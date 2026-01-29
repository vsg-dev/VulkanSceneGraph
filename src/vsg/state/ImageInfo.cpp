/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
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
    else if (format == VK_FORMAT_D16_UNORM)
    {
        traits.numBitsPerComponent = 16;
        traits.numComponents = 1;
        traits.size = 2;
    }
    else if (format == VK_FORMAT_D32_SFLOAT)
    {
        traits.numBitsPerComponent = 32;
        traits.numComponents = 1;
        traits.size = 4;
    }
    else if (format == VK_FORMAT_D16_UNORM_S8_UINT)
    {
        traits.packed = true;
        traits.numBitsPerComponent = 24;
        traits.numComponents = 1;
        traits.size = 3;
    }
    else if (format == VK_FORMAT_D24_UNORM_S8_UINT)
    {
        traits.packed = true;
        traits.numBitsPerComponent = 32;
        traits.numComponents = 1;
        traits.size = 4;
    }
    else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        traits.packed = true;
        traits.numBitsPerComponent = 40;
        traits.numComponents = 1;
        traits.size = 5;
    }
    else if (VK_FORMAT_A8B8G8R8_UNORM_PACK32 <= format && format <= VK_FORMAT_A8B8G8R8_SRGB_PACK32)
    {
        traits.packed = true;
        traits.numBitsPerComponent = 8;
        traits.numComponents = 4;
        traits.size = 4;
    }
    else if (VK_FORMAT_A2R10G10B10_UNORM_PACK32 <= format && format <= VK_FORMAT_A2B10G10R10_SINT_PACK32)
    {
        traits.packed = true;
        traits.numBitsPerComponent = 32; // Perhaps should be 10, except alpha is 2, will treat as one packed components
        traits.numComponents = 1;
        traits.size = 4;
    }
    else if (VK_FORMAT_B10G11R11_UFLOAT_PACK32 <= format && format <= VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
    {
        traits.packed = true;
        traits.numBitsPerComponent = 32; // Perhaps should be 10, except R is 11, will treat as one packed components
        traits.numComponents = 1;
        traits.size = 4;
    }
    else if (format == VK_FORMAT_X8_D24_UNORM_PACK32)
    {
        traits.packed = true;
        traits.numBitsPerComponent = 32;
        traits.numComponents = 1;
        traits.size = 4;
    }
    else if (VK_FORMAT_BC1_RGB_UNORM_BLOCK <= format && format <= VK_FORMAT_BC1_RGBA_SRGB_BLOCK)
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 64;
        traits.numComponents = 1;
        traits.size = 8;
    }
    else if (VK_FORMAT_BC2_UNORM_BLOCK <= format && format <= VK_FORMAT_BC3_SRGB_BLOCK)
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 128;
        traits.numComponents = 1;
        traits.size = 16;
    }
    else if (VK_FORMAT_BC4_UNORM_BLOCK <= format && format <= VK_FORMAT_BC4_SNORM_BLOCK)
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 64;
        traits.numComponents = 1;
        traits.size = 8;
    }
    else if (VK_FORMAT_BC5_UNORM_BLOCK <= format && format <= VK_FORMAT_BC5_SNORM_BLOCK)
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 128;
        traits.numComponents = 1;
        traits.size = 16;
    }
    else if (VK_FORMAT_BC6H_UFLOAT_BLOCK <= format && format <= VK_FORMAT_BC6H_SFLOAT_BLOCK)
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 128;
        traits.numComponents = 1;
        traits.size = 16;
    }
    else if (VK_FORMAT_BC7_UNORM_BLOCK <= format && format <= VK_FORMAT_BC7_SRGB_BLOCK)
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 128;
        traits.numComponents = 1;
        traits.size = 16;
    }
    else if ((VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK <= format && format <= VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK) ||
             (VK_FORMAT_EAC_R11_UNORM_BLOCK <= format && format <= VK_FORMAT_EAC_R11_SNORM_BLOCK))
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 64;
        traits.numComponents = 1;
        traits.size = 8;
    }
    else if ((VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK <= format && format <= VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK) ||
             (VK_FORMAT_EAC_R11G11_UNORM_BLOCK <= format && format <= VK_FORMAT_EAC_R11G11_SNORM_BLOCK))
    {
        traits.packed = true;
        traits.blockWidth = 4;
        traits.blockHeight = 4;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 128;
        traits.numComponents = 1;
        traits.size = 16;
    }
    else if (VK_FORMAT_ASTC_4x4_UNORM_BLOCK <= format && format <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
    {
        traits.packed = true;
        traits.blockDepth = 1;
        traits.numBitsPerComponent = 128;
        traits.numComponents = 1;
        traits.size = 16;

        switch (format)
        {
        case (VK_FORMAT_ASTC_4x4_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_4x4_SRGB_BLOCK):
            traits.blockWidth = 4;
            traits.blockHeight = 4;
            break;
        case (VK_FORMAT_ASTC_5x4_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_5x4_SRGB_BLOCK):
            traits.blockWidth = 5;
            traits.blockHeight = 4;
            break;
        case (VK_FORMAT_ASTC_5x5_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_5x5_SRGB_BLOCK):
            traits.blockWidth = 5;
            traits.blockHeight = 5;
            break;
        case (VK_FORMAT_ASTC_6x5_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_6x5_SRGB_BLOCK):
            traits.blockWidth = 6;
            traits.blockHeight = 5;
            break;
        case (VK_FORMAT_ASTC_6x6_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_6x6_SRGB_BLOCK):
            traits.blockWidth = 6;
            traits.blockHeight = 6;
            break;
        case (VK_FORMAT_ASTC_8x5_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_8x5_SRGB_BLOCK):
            traits.blockWidth = 8;
            traits.blockHeight = 5;
            break;
        case (VK_FORMAT_ASTC_8x6_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_8x6_SRGB_BLOCK):
            traits.blockWidth = 8;
            traits.blockHeight = 6;
            break;
        case (VK_FORMAT_ASTC_8x8_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_8x8_SRGB_BLOCK):
            traits.blockWidth = 8;
            traits.blockHeight = 8;
            break;
        case (VK_FORMAT_ASTC_10x5_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_10x5_SRGB_BLOCK):
            traits.blockWidth = 10;
            traits.blockHeight = 5;
            break;
        case (VK_FORMAT_ASTC_10x6_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_10x6_SRGB_BLOCK):
            traits.blockWidth = 10;
            traits.blockHeight = 6;
            break;
        case (VK_FORMAT_ASTC_10x8_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_10x8_SRGB_BLOCK):
            traits.blockWidth = 10;
            traits.blockHeight = 8;
            break;
        case (VK_FORMAT_ASTC_10x10_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_10x10_SRGB_BLOCK):
            traits.blockWidth = 10;
            traits.blockHeight = 10;
            break;
        case (VK_FORMAT_ASTC_12x10_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_12x10_SRGB_BLOCK):
            traits.blockWidth = 12;
            traits.blockHeight = 10;
            break;
        case (VK_FORMAT_ASTC_12x12_UNORM_BLOCK):
        case (VK_FORMAT_ASTC_12x12_SRGB_BLOCK):
            traits.blockWidth = 12;
            traits.blockHeight = 12;
            break;
        default:
            info("getFormatTraits(", format, ") not handled.");
            traits.blockWidth = 4;
            traits.blockHeight = 4;
            break;
        }
    }
    else
    {
        info("getFormatTraits(", format, ") not handled.");
    }

    return traits;
}

uint32_t vsg::computeNumMipMapLevels(const Data* data, const Sampler* sampler)
{
    uint32_t mipLevels = 1;
    if (sampler)
    {
        // clamp the mipLevels so that it's no larger than what the data dimensions support
        auto [width, height, depth, numLayers] = data->pixelExtents();
        uint32_t maxDimension = std::max({width, height, depth});
        if (sampler->maxLod == VK_LOD_CLAMP_NONE)
        {
            while ((1u << mipLevels) <= maxDimension)
            {
                ++mipLevels;
            }
        }
        else if (static_cast<uint32_t>(sampler->maxLod) > 1)
        {
            mipLevels = static_cast<uint32_t>(sampler->maxLod);
            while ((1u << (mipLevels - 1)) > maxDimension)
            {
                --mipLevels;
            }
        }

        bool generateMipmaps = (mipLevels > 1) && (data->properties.mipLevels <= 1);
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
            }
        }
        else
        {
            if (data->properties.mipLevels < mipLevels)
            {
                mipLevels = std::max(1u, static_cast<uint32_t>(data->properties.mipLevels));
            }
        }

        // vsg::info("vsg::computeNumMipMapLevels(data = ", data, ", sampler->maxLod = ", sampler->maxLod, ", data->properties.mipLevels = ", int(data->properties.mipLevels), ", mipLevels = ", mipLevels);
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

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer(sampler, rhs.sampler))) return result;
    if ((result = compare_pointer(imageView, rhs.imageView))) return result;
    return compare_value(imageLayout, rhs.imageLayout);
}

void ImageInfo::computeNumMipMapLevels()
{
    if (imageView && imageView->image && imageView->image->data)
    {
        const auto& image = imageView->image;
        const auto& data = image->data;
        image->mipLevels = vsg::computeNumMipMapLevels(data, sampler);
        bool generateMipmaps = (image->mipLevels > 1) && (data->properties.mipLevels <= 1);
        if (generateMipmaps) image->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
}

VkDeviceSize ImageInfo::computeDataSize() const
{
    if (imageView && imageView->image)
    {
        const auto& image = imageView->image;
        if (image->data) return image->data->computeValueCountIncludingMipmaps();
    }
    return 0;
}
