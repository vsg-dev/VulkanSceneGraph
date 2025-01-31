/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/state/Sampler.h>
#include <vsg/vk/Context.h>

using namespace vsg;

Sampler::Sampler()
{
}

Sampler::~Sampler()
{
}

int Sampler::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_region(flags, unnormalizedCoordinates, rhs.flags);
}

void Sampler::read(Input& input)
{
    input.readValue<uint32_t>("flags", flags);
    input.readValue<uint32_t>("minFilter", minFilter);
    input.readValue<uint32_t>("magFilter", magFilter);
    input.readValue<uint32_t>("mipmapMode", mipmapMode);
    input.readValue<uint32_t>("addressModeU", addressModeU);
    input.readValue<uint32_t>("addressModeV", addressModeV);
    input.readValue<uint32_t>("addressModeW", addressModeW);
    input.read("mipLodBias", mipLodBias);
    input.readValue<uint32_t>("anisotropyEnable", anisotropyEnable);
    input.read("maxAnisotropy", maxAnisotropy);
    input.readValue<uint32_t>("compareEnable", compareEnable);
    input.readValue<uint32_t>("compareOp", compareOp);
    input.read("minLod", minLod);
    input.read("maxLod", maxLod);
    input.readValue<uint32_t>("borderColor", borderColor);
    input.readValue<uint32_t>("unnormalizedCoordinates", unnormalizedCoordinates);
}

void Sampler::write(Output& output) const
{
    output.writeValue<uint32_t>("flags", flags);
    output.writeValue<uint32_t>("minFilter", minFilter);
    output.writeValue<uint32_t>("magFilter", magFilter);
    output.writeValue<uint32_t>("mipmapMode", mipmapMode);
    output.writeValue<uint32_t>("addressModeU", addressModeU);
    output.writeValue<uint32_t>("addressModeV", addressModeV);
    output.writeValue<uint32_t>("addressModeW", addressModeW);
    output.write("mipLodBias", mipLodBias);
    output.writeValue<uint32_t>("anisotropyEnable", anisotropyEnable);
    output.write("maxAnisotropy", maxAnisotropy);
    output.writeValue<uint32_t>("compareEnable", compareEnable);
    output.writeValue<uint32_t>("compareOp", compareOp);
    output.write("minLod", minLod);
    output.write("maxLod", maxLod);
    output.writeValue<uint32_t>("borderColor", borderColor);
    output.writeValue<uint32_t>("unnormalizedCoordinates", unnormalizedCoordinates);
}

void Sampler::compile(Context& context)
{
    if (_implementation[context.deviceID]) return;

    auto samplerInfo = context.scratchMemory->allocate<VkSamplerCreateInfo>();

    samplerInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo->pNext = nullptr;
    samplerInfo->flags = flags;
    samplerInfo->magFilter = magFilter;
    samplerInfo->minFilter = minFilter;
    samplerInfo->mipmapMode = mipmapMode;
    samplerInfo->addressModeU = addressModeU;
    samplerInfo->addressModeV = addressModeV;
    samplerInfo->addressModeW = addressModeW;
    samplerInfo->mipLodBias = mipLodBias;
    samplerInfo->anisotropyEnable = anisotropyEnable;
    samplerInfo->maxAnisotropy = maxAnisotropy;
    samplerInfo->compareEnable = compareEnable;
    samplerInfo->compareOp = compareOp;
    samplerInfo->minLod = minLod;
    samplerInfo->maxLod = maxLod;
    samplerInfo->borderColor = borderColor;
    samplerInfo->unnormalizedCoordinates = unnormalizedCoordinates;

    _implementation[context.deviceID] = Implementation::create(context.device, *samplerInfo);
}

Sampler::Implementation::Implementation(Device* device, const VkSamplerCreateInfo& createSamplerInfo) :
    _device(device)
{
    if (VkResult result = vkCreateSampler(*device, &createSamplerInfo, _device->getAllocationCallbacks(), &_sampler); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create VkSampler.", result};
    }
}

Sampler::Implementation::~Implementation()
{
    if (_sampler)
    {
        vkDestroySampler(*_device, _sampler, _device->getAllocationCallbacks());
    }
}
