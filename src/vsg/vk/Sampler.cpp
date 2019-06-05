/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Sampler.h>

using namespace vsg;

Sampler::Sampler(VkSampler sampler, const VkSamplerCreateInfo& info, Device* device, AllocationCallbacks* allocator) :
    _sampler(sampler),
    _info(info),
    _device(device),
    _allocator(allocator)
{
}

Sampler::~Sampler()
{
    if (_sampler)
    {
        vkDestroySampler(*_device, _sampler, _allocator);
    }
}

Sampler::Result Sampler::create(Device* device, const VkSamplerCreateInfo& createSamplerInfo, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::Sampler::create(...) failed to create vkSampler, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkSampler sampler;
    VkResult result = vkCreateSampler(*device, &createSamplerInfo, allocator, &sampler);
    if (result == VK_SUCCESS)
    {
        return Result(new Sampler(sampler, createSamplerInfo, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create vkSampler.", result);
    }
}

Sampler::Result Sampler::create(Device* device, AllocationCallbacks* allocator)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
#if 1
    // requires Logical device to have deviceFeatures.samplerAnisotropy = VK_TRUE; set when creating the vsg::Device
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
#else
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1;
#endif
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.pNext = nullptr;

    return create(device, samplerInfo, allocator);
}
