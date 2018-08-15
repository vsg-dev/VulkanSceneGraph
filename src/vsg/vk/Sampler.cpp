#include <vsg/vk/Sampler.h>

#include <iostream>

namespace vsg
{

Sampler::Sampler(VkSampler sampler, Device* device, AllocationCallbacks* allocator) :
    _sampler(sampler),
    _device(device),
    _allocator(allocator)
{
}

Sampler::~Sampler()
{
    if (_sampler)
    {
        std::cout<<"Calling vkDestroySampler"<<std::endl;
        vkDestroySampler(*_device, _sampler, *_allocator);
    }
}

Sampler::Result Sampler::create(Device* device, const VkSamplerCreateInfo& createSamplerInfo, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Sampler::Result("Error: vsg::Sampler::create(...) failed to create vkSampler, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkSampler sampler;
    VkResult result = vkCreateSampler(*device, &createSamplerInfo, *allocator, &sampler);
    if (result == VK_SUCCESS)
    {
        return new Sampler(sampler, device, allocator);
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
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
#if 1
    // requres Logical device to have deviceFeatures.samplerAnisotropy = VK_TRUE; set when creating the vsg::Deivce
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
#else
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1;
#endif
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    return create(device, samplerInfo, allocator);
}

}
