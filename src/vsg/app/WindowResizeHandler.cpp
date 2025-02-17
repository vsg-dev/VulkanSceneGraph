/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/app/WindowResizeHandler.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/State.h>

using namespace vsg;

UpdateGraphicsPipelines::UpdateGraphicsPipelines()
{
    overrideMask = MASK_ALL;
}

bool UpdateGraphicsPipelines::visit(const Object* object, uint32_t index)
{
    decltype(visited)::value_type objectIndex(object, index);
    if (visited.count(objectIndex) != 0) return false;
    visited.insert(objectIndex);
    return true;
}

void UpdateGraphicsPipelines::apply(vsg::Object& object)
{
    object.traverse(*this);
}

void UpdateGraphicsPipelines::apply(vsg::BindGraphicsPipeline& bindPipeline)
{
    if (!visit(&bindPipeline, context->viewID)) return;

    auto pipeline = bindPipeline.pipeline;
    if (pipeline)
    {
        pipeline->release(context->viewID);
        pipeline->compile(*context);
    }
}

void UpdateGraphicsPipelines::apply(vsg::View& view)
{
    if (!visit(&view, view.viewID)) return;

    context->viewID = view.viewID;
    context->defaultPipelineStates.emplace_back(view.camera->viewportState);

    view.traverse(*this);

    context->defaultPipelineStates.pop_back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WindowResizeHandler
//
WindowResizeHandler::WindowResizeHandler()
{
    overrideMask = MASK_ALL;
}

void WindowResizeHandler::scale_rect(VkRect2D& rect)
{
    int32_t edge_x = rect.offset.x + static_cast<int32_t>(rect.extent.width);
    int32_t edge_y = rect.offset.y + static_cast<int32_t>(rect.extent.height);

    rect.offset.x = scale_parameter(rect.offset.x, previous_extent.width, new_extent.width);
    rect.offset.y = scale_parameter(rect.offset.y, previous_extent.height, new_extent.height);
    rect.extent.width = static_cast<uint32_t>(scale_parameter(edge_x, previous_extent.width, new_extent.width) - rect.offset.x);
    rect.extent.height = static_cast<uint32_t>(scale_parameter(edge_y, previous_extent.height, new_extent.height) - rect.offset.y);
}

void WindowResizeHandler::scale_viewport(VkViewport& viewport)
{
    float scale_x =  static_cast<float>(new_extent.width) / static_cast<float>(previous_extent.width);
    float scale_y =  static_cast<float>(new_extent.height) / static_cast<float>(previous_extent.height);

    viewport.x *= scale_x;
    viewport.y *= scale_y;
    viewport.width *= scale_x;
    viewport.height *= scale_y;
}

bool WindowResizeHandler::visit(const Object* object, uint32_t index)
{
    decltype(visited)::value_type objectIndex(object, index);
    if (visited.count(objectIndex) != 0) return false;
    visited.insert(objectIndex);
    return true;
}

void WindowResizeHandler::apply(vsg::Object& object)
{
    object.traverse(*this);
}

void WindowResizeHandler::apply(ClearAttachments& clearAttachments)
{
    if (!visit(&clearAttachments)) return;

    for (auto& clearRect : clearAttachments.rects)
    {
        auto& rect = clearRect.rect;
        scale_rect(rect);
    }
}

void WindowResizeHandler::apply(vsg::View& view)
{
    if (!visit(&view)) return;

    if (!view.camera)
    {
        view.traverse(*this);
        return;
    }

    view.camera->projectionMatrix->changeExtent(previous_extent, new_extent);

    auto viewportState = view.camera->viewportState;

    size_t num_viewports = std::min(viewportState->viewports.size(), viewportState->scissors.size());
    for (size_t i = 0; i < num_viewports; ++i)
    {
        auto& viewport = viewportState->viewports[i];
        auto& scissor = viewportState->scissors[i];

        bool renderAreaMatches = (renderArea == scissor);

        scale_rect(scissor);
        scale_viewport(viewport);

        if (renderAreaMatches) renderArea = scissor;
    }

    view.traverse(*this);
}
