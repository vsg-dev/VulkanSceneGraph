/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/DynamicState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

DynamicState::DynamicState() :
    dynamicStates({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
{
}

DynamicState::DynamicState(const DynamicState& ds) :
    Inherit(ds),
    dynamicStates(ds.dynamicStates)
{
}

DynamicState::~DynamicState()
{
}

int DynamicState::compare(const Object& rhs_object) const
{
    int result = GraphicsPipelineState::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value_container(dynamicStates, rhs.dynamicStates);
}

void DynamicState::read(Input& input)
{
    GraphicsPipelineState::read(input);

    dynamicStates.resize(input.readValue<uint32_t>("NumDynamicStates"));
    for (auto& dynamicState : dynamicStates)
    {
        input.readValue<uint32_t>("value", dynamicState);
    }
}

void DynamicState::write(Output& output) const
{
    GraphicsPipelineState::write(output);

    output.writeValue<uint32_t>("NumDynamicStates", dynamicStates.size());
    for (auto& dynamicState : dynamicStates)
    {
        output.writeValue<uint32_t>("value", dynamicState);
    }
}

void DynamicState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto dynamicState = context.scratchMemory->allocate<VkPipelineDynamicStateCreateInfo>();

    dynamicState->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState->pNext = nullptr;
    dynamicState->flags = 0;
    dynamicState->dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState->pDynamicStates = dynamicStates.data();

    pipelineInfo.pDynamicState = dynamicState;
}
