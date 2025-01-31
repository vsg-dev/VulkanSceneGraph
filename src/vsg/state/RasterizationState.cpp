/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

RasterizationState::RasterizationState()
{
}

RasterizationState::RasterizationState(const RasterizationState& rs) :
    Inherit(rs),
    depthClampEnable(rs.depthClampEnable),
    rasterizerDiscardEnable(rs.rasterizerDiscardEnable),
    polygonMode(rs.polygonMode),
    cullMode(rs.cullMode),
    frontFace(rs.frontFace),
    depthBiasEnable(rs.depthBiasEnable),
    depthBiasConstantFactor(rs.depthBiasConstantFactor),
    depthBiasClamp(rs.depthBiasClamp),
    depthBiasSlopeFactor(rs.depthBiasSlopeFactor),
    lineWidth(rs.lineWidth)
{
}

RasterizationState::~RasterizationState()
{
}

int RasterizationState::compare(const Object& rhs_object) const
{
    int result = GraphicsPipelineState::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_region(depthClampEnable, lineWidth, rhs.depthClampEnable);
}

void RasterizationState::read(Input& input)
{
    GraphicsPipelineState::read(input);

    input.readValue<uint32_t>("depthClampEnable", depthClampEnable);
    input.readValue<uint32_t>("rasterizerDiscardEnable", rasterizerDiscardEnable);
    input.readValue<uint32_t>("polygonMode", polygonMode);
    input.readValue<uint32_t>("cullMode", cullMode);
    input.readValue<uint32_t>("frontFace", frontFace);
    input.readValue<uint32_t>("depthBiasEnable", depthBiasEnable);
    input.readValue<uint32_t>("depthBiasConstantFactor", depthBiasConstantFactor);
    input.read("depthBiasClamp", depthBiasClamp);
    input.read("depthBiasSlopeFactor", depthBiasSlopeFactor);
    input.read("lineWidth", lineWidth);
}

void RasterizationState::write(Output& output) const
{
    GraphicsPipelineState::write(output);

    output.writeValue<uint32_t>("depthClampEnable", depthClampEnable);
    output.writeValue<uint32_t>("rasterizerDiscardEnable", rasterizerDiscardEnable);
    output.writeValue<uint32_t>("polygonMode", polygonMode);
    output.writeValue<uint32_t>("cullMode", cullMode);
    output.writeValue<uint32_t>("frontFace", frontFace);
    output.writeValue<uint32_t>("depthBiasEnable", depthBiasEnable);
    output.writeValue<uint32_t>("depthBiasConstantFactor", depthBiasConstantFactor);
    output.write("depthBiasClamp", depthBiasClamp);
    output.write("depthBiasSlopeFactor", depthBiasSlopeFactor);
    output.write("lineWidth", lineWidth);
}

void RasterizationState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto rasteratizationState = context.scratchMemory->allocate<VkPipelineRasterizationStateCreateInfo>();

    rasteratizationState->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasteratizationState->pNext = nullptr;
    rasteratizationState->flags = 0;

    rasteratizationState->depthClampEnable = depthClampEnable;
    rasteratizationState->rasterizerDiscardEnable = rasterizerDiscardEnable;
    rasteratizationState->polygonMode = polygonMode;
    rasteratizationState->cullMode = cullMode;
    rasteratizationState->frontFace = frontFace;
    rasteratizationState->depthBiasEnable = depthBiasEnable;
    rasteratizationState->depthBiasConstantFactor = depthBiasConstantFactor;
    rasteratizationState->depthBiasClamp = depthBiasClamp;
    rasteratizationState->depthBiasSlopeFactor = depthBiasSlopeFactor;
    rasteratizationState->lineWidth = lineWidth;

    pipelineInfo.pRasterizationState = rasteratizationState;
}
