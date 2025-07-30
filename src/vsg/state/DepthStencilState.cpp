/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

DepthStencilState::DepthStencilState()
{
}

DepthStencilState::DepthStencilState(const DepthStencilState& dss) :
    Inherit(dss),
    depthTestEnable(dss.depthTestEnable),
    depthWriteEnable(dss.depthWriteEnable),
    depthCompareOp(dss.depthCompareOp),
    depthBoundsTestEnable(dss.depthBoundsTestEnable),
    stencilTestEnable(dss.stencilTestEnable),
    front(dss.front),
    back(dss.back),
    minDepthBounds(dss.minDepthBounds),
    maxDepthBounds(dss.maxDepthBounds)
{
}

DepthStencilState::~DepthStencilState()
{
}

int DepthStencilState::compare(const Object& rhs_object) const
{
    int result = GraphicsPipelineState::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_region(depthTestEnable, maxDepthBounds, rhs.depthTestEnable);
}

void DepthStencilState::read(Input& input)
{
    GraphicsPipelineState::read(input);

    input.readValue<uint32_t>("depthTestEnable", depthTestEnable);
    input.readValue<uint32_t>("depthWriteEnable", depthWriteEnable);
    input.readValue<uint32_t>("depthCompareOp", depthCompareOp);
    input.readValue<uint32_t>("depthBoundsTestEnable", depthBoundsTestEnable);
    input.readValue<uint32_t>("stencilTestEnable", stencilTestEnable);

    input.readValue<uint32_t>("front.failOp", front.failOp);
    input.readValue<uint32_t>("front.passOp", front.passOp);
    input.readValue<uint32_t>("front.depthFailOp", front.depthFailOp);
    input.readValue<uint32_t>("front.compareOp", front.compareOp);
    input.read("front.compareMask", front.compareMask);
    input.read("front.writeMask", front.writeMask);
    input.read("front.reference", front.reference);

    input.readValue<uint32_t>("back.failOp", back.failOp);
    input.readValue<uint32_t>("back.passOp", back.passOp);
    input.readValue<uint32_t>("back.depthFailOp", back.depthFailOp);
    input.readValue<uint32_t>("back.compareOp", back.compareOp);
    input.read("back.compareMask", back.compareMask);
    input.read("back.writeMask", back.writeMask);
    input.read("back.reference", back.reference);

    input.read("minDepthBounds", minDepthBounds);
    input.read("maxDepthBounds", maxDepthBounds);
}

void DepthStencilState::write(Output& output) const
{
    GraphicsPipelineState::write(output);

    output.writeValue<uint32_t>("depthTestEnable", depthTestEnable);
    output.writeValue<uint32_t>("depthWriteEnable", depthWriteEnable);
    output.writeValue<uint32_t>("depthCompareOp", depthCompareOp);
    output.writeValue<uint32_t>("depthBoundsTestEnable", depthBoundsTestEnable);
    output.writeValue<uint32_t>("stencilTestEnable", stencilTestEnable);

    output.writeValue<uint32_t>("front.failOp", front.failOp);
    output.writeValue<uint32_t>("front.passOp", front.passOp);
    output.writeValue<uint32_t>("front.depthFailOp", front.depthFailOp);
    output.writeValue<uint32_t>("front.compareOp", front.compareOp);
    output.write("front.compareMask", front.compareMask);
    output.write("front.writeMask", front.writeMask);
    output.write("front.reference", front.reference);

    output.writeValue<uint32_t>("back.failOp", back.failOp);
    output.writeValue<uint32_t>("back.passOp", back.passOp);
    output.writeValue<uint32_t>("back.depthFailOp", back.depthFailOp);
    output.writeValue<uint32_t>("back.compareOp", back.compareOp);
    output.write("back.compareMask", back.compareMask);
    output.write("back.writeMask", back.writeMask);
    output.write("back.reference", back.reference);

    output.write("minDepthBounds", minDepthBounds);
    output.write("maxDepthBounds", maxDepthBounds);
}

void DepthStencilState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto depthStencilState = context.scratchMemory->allocate<VkPipelineDepthStencilStateCreateInfo>();

    depthStencilState->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState->pNext = nullptr;
    depthStencilState->flags = 0;

    depthStencilState->depthTestEnable = depthTestEnable;
    depthStencilState->depthWriteEnable = depthWriteEnable;
    depthStencilState->depthCompareOp = depthCompareOp;
    depthStencilState->depthBoundsTestEnable = depthBoundsTestEnable;
    depthStencilState->stencilTestEnable = stencilTestEnable;
    depthStencilState->front = front;
    depthStencilState->back = back;
    depthStencilState->minDepthBounds = minDepthBounds;
    depthStencilState->maxDepthBounds = maxDepthBounds;

    pipelineInfo.pDepthStencilState = depthStencilState;
}
