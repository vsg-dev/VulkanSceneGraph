/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/ViewportState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

ViewportState::ViewportState()
{
    slot = 8;
}

ViewportState::ViewportState(const ViewportState& vs) :
    Inherit(vs),
    viewports(vs.viewports),
    scissors(vs.scissors)
{
}

ViewportState::ViewportState(const VkExtent2D& extent) :
    ViewportState()
{
    set(0, 0, extent.width, extent.height);
}

ViewportState::ViewportState(int32_t x, int32_t y, uint32_t width, uint32_t height) :
    ViewportState()
{
    set(x, y, width, height);
}

ViewportState::~ViewportState()
{
}

int ViewportState::compare(const Object& rhs_object) const
{
    int result = GraphicsPipelineState::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value_container(viewports, rhs.viewports))) return result;
    return compare_value_container(scissors, rhs.scissors);
}

void ViewportState::set(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    viewports.resize(1);
    scissors.resize(1);

    VkViewport& viewport = viewports[0];
    viewport.x = static_cast<float>(x);
    viewport.y = static_cast<float>(y);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D& scissor = scissors[0];
    scissor.offset = VkOffset2D{x, y};
    scissor.extent = VkExtent2D{width, height};
}

void ViewportState::read(Input& input)
{
    GraphicsPipelineState::read(input);

    viewports.resize(input.readValue<uint32_t>("viewports"));
    for (auto& viewport : viewports)
    {
        input.read("x", viewport.x);
        input.read("y", viewport.y);
        input.read("width", viewport.width);
        input.read("height", viewport.height);
        input.read("minDepth", viewport.minDepth);
        input.read("maxDepth", viewport.maxDepth);
    }

    scissors.resize(input.readValue<uint32_t>("scissors"));
    for (auto& scissor : scissors)
    {
        input.read("x", scissor.offset.x);
        input.read("y", scissor.offset.y);
        input.read("width", scissor.extent.width);
        input.read("height", scissor.extent.height);
    }
}

void ViewportState::write(Output& output) const
{
    GraphicsPipelineState::write(output);

    output.writeValue<uint32_t>("viewports", viewports.size());
    for (auto& viewport : viewports)
    {
        output.write("x", viewport.x);
        output.write("y", viewport.y);
        output.write("width", viewport.width);
        output.write("height", viewport.height);
        output.write("minDepth", viewport.minDepth);
        output.write("maxDepth", viewport.maxDepth);
    }

    output.writeValue<uint32_t>("scissors", scissors.size());
    for (auto& scissor : scissors)
    {
        output.write("x", scissor.offset.x);
        output.write("y", scissor.offset.y);
        output.write("width", scissor.extent.width);
        output.write("height", scissor.extent.height);
    }
}

void ViewportState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto viewportState = context.scratchMemory->allocate<VkPipelineViewportStateCreateInfo>();

    viewportState->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState->pNext = nullptr;
    viewportState->flags = 0;

    viewportState->viewportCount = static_cast<uint32_t>(viewports.size());
    viewportState->pViewports = viewports.data();
    viewportState->scissorCount = static_cast<uint32_t>(scissors.size());
    viewportState->pScissors = scissors.data();

    pipelineInfo.pViewportState = viewportState;
}

void ViewportState::record(CommandBuffer& commandBuffer) const
{
    vkCmdSetScissor(commandBuffer, 0, static_cast<uint32_t>(scissors.size()), scissors.data());
    vkCmdSetViewport(commandBuffer, 0, static_cast<uint32_t>(viewports.size()), viewports.data());
}

VkViewport& ViewportState::getViewport()
{
    if (viewports.empty())
    {
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 0.0f;
        viewport.height = 0.0f;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewports.push_back(viewport);
    }
    return viewports[0];
}

VkRect2D& ViewportState::getScissor()
{
    if (scissors.empty())
    {
        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = {0, 0};
        scissors.push_back(scissor);
    }
    return scissors[0];
}
