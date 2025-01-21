/* <editor-fold desc="MIT License">

Copyright(c) 2024 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/utils/CoordinateSpace.h>

using namespace vsg;

VkFormat vsg::uNorm_to_sRGB(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM: {
        format = VK_FORMAT_R8_SRGB;
        break;
    }
    case VK_FORMAT_R8G8_UNORM: {
        format = VK_FORMAT_R8G8_SRGB;
        break;
    }
    case VK_FORMAT_R8G8B8_UNORM: {
        format = VK_FORMAT_R8G8B8_SRGB;
        break;
    }
    case VK_FORMAT_B8G8R8_UNORM: {
        format = VK_FORMAT_B8G8R8_SRGB;
        break;
    }
    case VK_FORMAT_R8G8B8A8_UNORM: {
        format = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    }
    case VK_FORMAT_B8G8R8A8_UNORM: {
        format = VK_FORMAT_B8G8R8A8_SRGB;
        break;
    }
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32: {
        format = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
        break;
    }
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK: {
        format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: {
        format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_BC2_UNORM_BLOCK: {
        format = VK_FORMAT_BC2_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_BC3_UNORM_BLOCK: {
        format = VK_FORMAT_BC3_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_BC7_UNORM_BLOCK: {
        format = VK_FORMAT_BC7_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: {
        format = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: {
        format = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: {
        format = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: {
        format = VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
        break;
    }
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
        break;
    }
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
        break;
    }
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
        break;
    }
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;
        break;
    }
    default:
        break;
    }
    return format;
}

VkFormat vsg::sRGB_to_uNorm(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_SRGB: {
        format = VK_FORMAT_R8_UNORM;
        break;
    }
    case VK_FORMAT_R8G8_SRGB: {
        format = VK_FORMAT_R8G8_UNORM;
        break;
    }
    case VK_FORMAT_R8G8B8_SRGB: {
        format = VK_FORMAT_R8G8B8_UNORM;
        break;
    }
    case VK_FORMAT_B8G8R8_SRGB: {
        format = VK_FORMAT_B8G8R8_UNORM;
        break;
    }
    case VK_FORMAT_R8G8B8A8_SRGB: {
        format = VK_FORMAT_R8G8B8A8_UNORM;
        break;
    }
    case VK_FORMAT_B8G8R8A8_SRGB: {
        format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    }
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32: {
        format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
        break;
    }
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK: {
        format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: {
        format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_BC2_SRGB_BLOCK: {
        format = VK_FORMAT_BC2_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_BC3_SRGB_BLOCK: {
        format = VK_FORMAT_BC3_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_BC7_SRGB_BLOCK: {
        format = VK_FORMAT_BC7_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: {
        format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: {
        format = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: {
        format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: {
        format = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        break;
    }
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
        break;
    }
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
        break;
    }
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
        break;
    }
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: {
        format = VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
        break;
    }
    default:
        break;
    }
    return format;
}
