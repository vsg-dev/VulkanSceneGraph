/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/View.h>
#include <vsg/viewer/Viewer.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

CompileTraversal::CompileTraversal(const CompileTraversal& ct) :
    Inherit(ct)
{
    for (auto& context : ct.contexts)
    {
        contexts.push_back(Context::create(*context));
    }
}

CompileTraversal::CompileTraversal(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements)
{
    add(device, resourceRequirements);
}

CompileTraversal::CompileTraversal(ref_ptr<Window> window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    add(window, viewport, resourceRequirements);
}

CompileTraversal::CompileTraversal(Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    add(viewer, resourceRequirements);
}

CompileTraversal::~CompileTraversal()
{
}

void CompileTraversal::add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements)
{
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto context = Context::create(device, resourceRequirements);
    context->commandPool = CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily);
    contexts.push_back(context);
}

void CompileTraversal::add(ref_ptr<Window> window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    auto device = window->getOrCreateDevice();
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto context = Context::create(device, resourceRequirements);
    context->renderPass = window->getOrCreateRenderPass();
    context->commandPool = CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily);

    if (viewport) context->defaultPipelineStates.emplace_back(viewport);
    if (window->framebufferSamples() != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(MultisampleState::create(window->framebufferSamples()));

    contexts.push_back(context);
}

void CompileTraversal::add(ref_ptr<Window> window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    std::cout << "CompileTraversal::add(window =" << window << ", view = " << view << ")" << std::endl;

    auto device = window->getOrCreateDevice();
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto context = Context::create(device, resourceRequirements);
    context->renderPass = window->getOrCreateRenderPass();
    context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily);

    if (window->framebufferSamples() != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(vsg::MultisampleState::create(window->framebufferSamples()));

    auto viewportState = view->camera->viewportState;
    if (viewportState) context->defaultPipelineStates.emplace_back(viewportState);

    context->view = view.get();
    context->viewID = view->viewID;
    context->viewDependentState = view->viewDependentState;

    contexts.push_back(context);
}

void CompileTraversal::add(Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    struct AddViews : public Visitor
    {
        CompileTraversal* ct = nullptr;
        const ResourceRequirements& resourceRequirements;
        AddViews(CompileTraversal* in_ct, const ResourceRequirements& in_rr) :
            ct(in_ct), resourceRequirements(in_rr){};

        std::stack<ref_ptr<Window>> windowStack;

        void apply(Object& object) override
        {
            object.traverse(*this);
        }

        void apply(RenderGraph& rg) override
        {
            windowStack.emplace(rg.window);

            rg.traverse(*this);

            windowStack.pop();
        }

        void apply(View& view) override
        {
            if (!windowStack.empty())
            {
                ct->add(windowStack.top(), ref_ptr<View>(&view), resourceRequirements);
            }
        }
    } addViews(this, resourceRequirements);

    for (auto& task : viewer.recordAndSubmitTasks)
    {
        for (auto& cg : task->commandGraphs)
        {
            cg->accept(addViews);
        }
    }
}

void CompileTraversal::apply(Object& object)
{
    object.traverse(*this);
}

void CompileTraversal::apply(Command& command)
{
    for (auto& context : contexts)
    {
        command.compile(*context);
    }
}

void CompileTraversal::apply(Commands& commands)
{
    for (auto& context : contexts)
    {
        commands.compile(*context);
    }
}

void CompileTraversal::apply(StateGroup& stateGroup)
{
    for (auto& context : contexts)
    {
        stateGroup.compile(*context);
    }
    stateGroup.traverse(*this);
}

void CompileTraversal::apply(Geometry& geometry)
{
    for (auto& context : contexts)
    {
        geometry.compile(*context);
    }
    geometry.traverse(*this);
}

void CompileTraversal::apply(CommandGraph& commandGraph)
{
    if (commandGraph.window)
    {
        for (auto& context : contexts)
        {
            context->renderPass = commandGraph.window->getOrCreateRenderPass();

            context->defaultPipelineStates.push_back(ViewportState::create(commandGraph.window->extent2D()));

            if (commandGraph.window->framebufferSamples() != VK_SAMPLE_COUNT_1_BIT)
            {
                ref_ptr<MultisampleState> defaultMsState = MultisampleState::create(commandGraph.window->framebufferSamples());
                context->overridePipelineStates.push_back(defaultMsState);
            }

            // save previous states to be restored after traversal
            auto previousDefaultPipelineStates = context->defaultPipelineStates;
            auto previousOverridePipelineStates = context->overridePipelineStates;

            commandGraph.traverse(*this);

            // restore previous values
            context->defaultPipelineStates = previousDefaultPipelineStates;
            context->overridePipelineStates = previousOverridePipelineStates;
        }
    }
    else
    {
        commandGraph.traverse(*this);
    }
}

void CompileTraversal::apply(RenderGraph& renderGraph)
{
    for (auto& context : contexts)
    {
        context->renderPass = renderGraph.getRenderPass();

        // save previous states to be restored after traversal
        auto previousDefaultPipelineStates = context->defaultPipelineStates;
        auto previousOverridePipelineStates = context->overridePipelineStates;

        if (renderGraph.window)
        {
            context->defaultPipelineStates.push_back(ViewportState::create(renderGraph.window->extent2D()));
        }
        else if (renderGraph.framebuffer)
        {
            VkExtent2D extent{renderGraph.framebuffer->width(), renderGraph.framebuffer->height()};
            context->defaultPipelineStates.push_back(ViewportState::create(extent));
        }

        if (context->renderPass && context->renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT)
        {
            ref_ptr<MultisampleState> defaultMsState = MultisampleState::create(context->renderPass->maxSamples);
            context->overridePipelineStates.push_back(defaultMsState);
        }

        renderGraph.traverse(*this);

        // restore previous values
        context->defaultPipelineStates = previousDefaultPipelineStates;
        context->overridePipelineStates = previousOverridePipelineStates;
    }
}

void CompileTraversal::apply(View& view)
{
    for (auto& context : contexts)
    {
        context->viewID = view.viewID;
        context->viewDependentState = view.viewDependentState.get();
        if (view.viewDependentState) view.viewDependentState->compile(*context);

        if (view.camera && view.camera->viewportState)
        {
            context->defaultPipelineStates.emplace_back(view.camera->viewportState);

            view.traverse(*this);

            context->defaultPipelineStates.pop_back();
        }
        else
        {
            view.traverse(*this);
        }
    }
}

bool CompileTraversal::record()
{
    bool recorded = false;
    for (auto& context : contexts)
    {
        if (context->record()) recorded = true;
    }
    return recorded;
}

void CompileTraversal::waitForCompletion()
{
    for (auto& context : contexts)
    {
        context->waitForCompletion();
    }
}
