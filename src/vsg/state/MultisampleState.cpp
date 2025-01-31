/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

MultisampleState::MultisampleState(VkSampleCountFlagBits samples) :
    rasterizationSamples(samples)
{
}

MultisampleState::MultisampleState(const MultisampleState& ms) :
    Inherit(ms),
    rasterizationSamples(ms.rasterizationSamples),
    sampleShadingEnable(ms.sampleShadingEnable),
    minSampleShading(ms.minSampleShading),
    sampleMasks(ms.sampleMasks),
    alphaToCoverageEnable(ms.alphaToCoverageEnable),
    alphaToOneEnable(ms.alphaToOneEnable)
{
}

MultisampleState::~MultisampleState()
{
}

int MultisampleState::compare(const Object& rhs_object) const
{
    int result = GraphicsPipelineState::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(rasterizationSamples, rhs.rasterizationSamples))) return result;
    if ((result = compare_value(sampleShadingEnable, rhs.sampleShadingEnable))) return result;
    if ((result = compare_value(minSampleShading, rhs.minSampleShading))) return result;
    if ((result = compare_value_container(sampleMasks, rhs.sampleMasks))) return result;
    if ((result = compare_value(alphaToCoverageEnable, rhs.alphaToCoverageEnable))) return result;
    return compare_value(alphaToOneEnable, rhs.alphaToOneEnable);
}

void MultisampleState::read(Input& input)
{
    GraphicsPipelineState::read(input);

    input.readValue<uint32_t>("rasterizationSamples", rasterizationSamples);
    input.readValue<uint32_t>("sampleShadingEnable", sampleShadingEnable);
    input.read("minSampleShading", minSampleShading);

    if (input.version_greater_equal(0, 7, 3))
        sampleMasks.resize(input.readValue<uint32_t>("sampleMasks"));
    else
        sampleMasks.resize(input.readValue<uint32_t>("NumSampleMask"));

    for (auto& value : sampleMasks)
    {
        input.readValue<uint32_t>("value", value);
    }

    input.readValue<uint32_t>("alphaToCoverageEnable", alphaToCoverageEnable);
    input.readValue<uint32_t>("alphaToOneEnable", alphaToOneEnable);
}

void MultisampleState::write(Output& output) const
{
    GraphicsPipelineState::write(output);

    output.writeValue<uint32_t>("rasterizationSamples", rasterizationSamples);
    output.writeValue<uint32_t>("sampleShadingEnable", sampleShadingEnable);
    output.write("minSampleShading", minSampleShading);

    if (output.version_greater_equal(0, 7, 3))
        output.writeValue<uint32_t>("sampleMasks", sampleMasks.size());
    else
        output.writeValue<uint32_t>("NumSampleMask", sampleMasks.size());

    for (auto& value : sampleMasks)
    {
        output.writeValue<uint32_t>("value", value);
    }

    output.writeValue<uint32_t>("alphaToCoverageEnable", alphaToCoverageEnable);
    output.writeValue<uint32_t>("alphaToOneEnable", alphaToOneEnable);
}

void MultisampleState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto multisampleState = context.scratchMemory->allocate<VkPipelineMultisampleStateCreateInfo>();

    multisampleState->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState->pNext = nullptr;
    multisampleState->flags = 0;

    multisampleState->rasterizationSamples = rasterizationSamples;
    multisampleState->sampleShadingEnable = sampleShadingEnable;
    multisampleState->minSampleShading = minSampleShading;
    multisampleState->pSampleMask = sampleMasks.empty() ? nullptr : sampleMasks.data();
    multisampleState->alphaToCoverageEnable = alphaToCoverageEnable;
    multisampleState->alphaToOneEnable = alphaToOneEnable;

    pipelineInfo.pMultisampleState = multisampleState;
}
