/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/state/Sampler.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/io/Options.h>

using namespace vsg;

Sampler::Sampler()
{
    // set default sampler info
    _samplerInfo = {};
    _samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    _samplerInfo.pNext = nullptr;
    _samplerInfo.flags = 0;
    _samplerInfo.minFilter = VK_FILTER_LINEAR;
    _samplerInfo.magFilter = VK_FILTER_LINEAR;
    _samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    _samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    _samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
#if 1
    // requires Logical device to have deviceFeatures.samplerAnisotropy = VK_TRUE; set when creating the vsg::Device
    _samplerInfo.anisotropyEnable = VK_TRUE;
    _samplerInfo.maxAnisotropy = 16;
#else
    _samplerInfo.anisotropyEnable = VK_FALSE;
    _samplerInfo.maxAnisotropy = 1;
#endif
    _samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    _samplerInfo.unnormalizedCoordinates = VK_FALSE;
    _samplerInfo.compareEnable = VK_FALSE;
    _samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

Sampler::~Sampler()
{
}

void Sampler::read(Input& input)
{
    input.readValue<uint32_t>("flags", _samplerInfo.flags);
    input.readValue<uint32_t>("minFilter", _samplerInfo.minFilter);
    input.readValue<uint32_t>("magFilter", _samplerInfo.magFilter);
    input.readValue<uint32_t>("mipmapMode", _samplerInfo.mipmapMode);
    input.readValue<uint32_t>("addressModeU", _samplerInfo.addressModeU);
    input.readValue<uint32_t>("addressModeV", _samplerInfo.addressModeV);
    input.readValue<uint32_t>("addressModeW", _samplerInfo.addressModeW);
    input.read("mipLodBias", _samplerInfo.mipLodBias);
    input.readValue<uint32_t>("anisotropyEnable", _samplerInfo.anisotropyEnable);
    input.read("maxAnisotropy", _samplerInfo.maxAnisotropy);
    input.readValue<uint32_t>("compareEnable", _samplerInfo.compareEnable);
    input.readValue<uint32_t>("compareOp", _samplerInfo.compareOp);
    input.read("minLod", _samplerInfo.minLod);
    input.read("maxLod", _samplerInfo.maxLod);
    input.readValue<uint32_t>("borderColor", _samplerInfo.borderColor);
    input.readValue<uint32_t>("unnormalizedCoordinates", _samplerInfo.unnormalizedCoordinates);
}

void Sampler::write(Output& output) const
{
    output.writeValue<uint32_t>("flags", _samplerInfo.flags);
    output.writeValue<uint32_t>("minFilter", _samplerInfo.minFilter);
    output.writeValue<uint32_t>("magFilter", _samplerInfo.magFilter);
    output.writeValue<uint32_t>("mipmapMode", _samplerInfo.mipmapMode);
    output.writeValue<uint32_t>("addressModeU", _samplerInfo.addressModeU);
    output.writeValue<uint32_t>("addressModeV", _samplerInfo.addressModeV);
    output.writeValue<uint32_t>("addressModeW", _samplerInfo.addressModeW);
    output.write("mipLodBias", _samplerInfo.mipLodBias);
    output.writeValue<uint32_t>("anisotropyEnable", _samplerInfo.anisotropyEnable);
    output.write("maxAnisotropy", _samplerInfo.maxAnisotropy);
    output.writeValue<uint32_t>("compareEnable", _samplerInfo.compareEnable);
    output.writeValue<uint32_t>("compareOp", _samplerInfo.compareOp);
    output.write("minLod", _samplerInfo.minLod);
    output.write("maxLod", _samplerInfo.maxLod);
    output.writeValue<uint32_t>("borderColor", _samplerInfo.borderColor);
    output.writeValue<uint32_t>("unnormalizedCoordinates", _samplerInfo.unnormalizedCoordinates);
}

void Sampler::compile(Context& context)
{
    if (_implementation[context.deviceID]) return;

    _implementation[context.deviceID] = Implementation::create(context.device, _samplerInfo);
}

Sampler::Implementation::Implementation(Device* device, const VkSamplerCreateInfo& createSamplerInfo, AllocationCallbacks* allocator) :
    _device(device),
    _allocator(allocator)
{
    if (VkResult result = vkCreateSampler(*device, &createSamplerInfo, allocator, &_sampler); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkSampler.", result};
    }
}

Sampler::Implementation::~Implementation()
{
    if (_sampler)
    {
        vkDestroySampler(*_device, _sampler, _allocator);
    }
}
