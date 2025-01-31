/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/TessellationState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

TessellationState::TessellationState(uint32_t in_patchControlPoints) :
    patchControlPoints(in_patchControlPoints)
{
}

TessellationState::TessellationState(const TessellationState& ts) :
    Inherit(ts),
    patchControlPoints(ts.patchControlPoints)
{
}

TessellationState::~TessellationState()
{
}

int TessellationState::compare(const Object& rhs_object) const
{
    int result = GraphicsPipelineState::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value(patchControlPoints, rhs.patchControlPoints);
}

void TessellationState::read(Input& input)
{
    GraphicsPipelineState::read(input);

    input.read("patchControlPoints", patchControlPoints);
}

void TessellationState::write(Output& output) const
{
    GraphicsPipelineState::write(output);

    output.write("patchControlPoints", patchControlPoints);
}

void TessellationState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{

    auto tessellationState = context.scratchMemory->allocate<VkPipelineTessellationStateCreateInfo>();

    tessellationState->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState->pNext = nullptr;
    tessellationState->flags = 0;
    tessellationState->patchControlPoints = patchControlPoints;

    pipelineInfo.pTessellationState = tessellationState;
}
