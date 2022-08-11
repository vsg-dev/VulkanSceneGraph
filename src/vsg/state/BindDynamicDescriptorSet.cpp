/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/BindDynamicDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

void BindDynamicDescriptorSet::compile(Context& context)
{
    if (_dynamicDescriptors.empty())
    {
        for (auto& descriptor : descriptorSet->descriptors)
        {
            auto type = descriptor->descriptorType;
            if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            {
                auto descriptorBuffer = descriptor->cast<DescriptorBuffer>();
                if (descriptorBuffer)
                    _dynamicDescriptors.emplace_back(descriptorBuffer);
            }
        }
        // descriptor offsets must be specified in binding order
        std::sort(_dynamicDescriptors.begin(), _dynamicDescriptors.end(), [](const auto& lhs, const auto& rhs) -> auto { return lhs->dstBinding < rhs->dstBinding; });
    }

    Inherit::compile(context);
}

void BindDynamicDescriptorSet::record(CommandBuffer& commandBuffer) const
{
    std::vector<uint32_t> offsets;
    for (auto& descriptor : _dynamicDescriptors)
    {
        for (auto& bi : descriptor->bufferInfoList)
        {
            auto offset = 0;
            // if we have a parent, treat the BufferInfo's offset as a dynamic offset, otherwise leave offset unchanged
            // note, to use dynamic offsets the BufferInfo must have a parent, otherwise the BufferInfo's destructor will release an incorrect buffer range on destruction
            if (bi->parent)
                offset = bi->offset - bi->parent->offset;
            offsets.push_back(offset);
        }
    }

    auto& vkd = _vulkanData[commandBuffer.deviceID];
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, vkd._vkPipelineLayout, firstSet, 1, &(vkd._vkDescriptorSet), static_cast<uint32_t>(offsets.size()), offsets.data());
}
