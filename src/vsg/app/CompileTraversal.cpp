/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CompileTraversal.h>

#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/SecondaryCommandGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

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
    overrideMask = vsg::MASK_ALL;
    add(device, resourceRequirements);
}

CompileTraversal::CompileTraversal(Window& window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    overrideMask = vsg::MASK_ALL;
    add(window, viewport, resourceRequirements);
}

CompileTraversal::CompileTraversal(const Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    overrideMask = vsg::MASK_ALL;
    add(viewer, resourceRequirements);
}

CompileTraversal::~CompileTraversal()
{
}

void CompileTraversal::add(ref_ptr<Device> device, ref_ptr<TransferTask> transferTask, const ResourceRequirements& resourceRequirements)
{
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(queueFlags);
    auto context = Context::create(device, resourceRequirements);
    context->instrumentation = instrumentation;
    context->commandPool = CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily, queueFamilyIndex);
    context->transferTask = transferTask;
    contexts.push_back(context);
}

void CompileTraversal::add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements)
{
    add(device, nullptr, resourceRequirements);
}

void CompileTraversal::add(Window& window, ref_ptr<TransferTask> transferTask, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    auto device = window.getOrCreateDevice();
    auto renderPass = window.getOrCreateRenderPass();
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(queueFlags);
    auto context = Context::create(device, resourceRequirements);
    context->instrumentation = instrumentation;
    context->renderPass = renderPass;
    context->commandPool = CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily, queueFamilyIndex);
    context->transferTask = transferTask;

    if (viewport)
        context->defaultPipelineStates.emplace_back(viewport);
    else
        context->defaultPipelineStates.emplace_back(vsg::ViewportState::create(window.extent2D()));

    if (renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(MultisampleState::create(renderPass->maxSamples));

    contexts.push_back(context);
}

void CompileTraversal::add(Window& window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    add(window, nullptr, viewport, resourceRequirements);
}

void CompileTraversal::add(Window& window, ref_ptr<TransferTask> transferTask, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    auto device = window.getOrCreateDevice();
    auto renderPass = window.getOrCreateRenderPass();
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(queueFlags);
    auto context = Context::create(device, resourceRequirements);
    context->instrumentation = instrumentation;
    context->renderPass = renderPass;
    context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily, queueFamilyIndex);
    context->transferTask = transferTask;

    if (renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(vsg::MultisampleState::create(renderPass->maxSamples));

    context->overridePipelineStates.insert(context->overridePipelineStates.end(), view->overridePipelineStates.begin(), view->overridePipelineStates.end());

    if (view->camera && view->camera->viewportState)
        context->defaultPipelineStates.emplace_back(view->camera->viewportState);
    else
        context->defaultPipelineStates.emplace_back(vsg::ViewportState::create(window.extent2D()));

    context->view = view.get();
    context->viewID = view->viewID;
    context->viewDependentState = view->viewDependentState;

    contexts.push_back(context);

    if (view->viewDependentState) addViewDependentState(*(view->viewDependentState), device, transferTask, resourceRequirements);
}

void CompileTraversal::add(Window& window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    add(window, nullptr, view, resourceRequirements);
}

void CompileTraversal::add(Framebuffer& framebuffer, ref_ptr<TransferTask> transferTask, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    ref_ptr<Device> device(framebuffer.getDevice());
    auto context = Context::create(device, resourceRequirements);
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    context->instrumentation = instrumentation;
    context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily, queueFamilyIndex);
    context->transferTask = transferTask;

    add(context, framebuffer, transferTask, view, resourceRequirements);
}

void CompileTraversal::add(ref_ptr<Context> context, Framebuffer& framebuffer, ref_ptr<TransferTask> transferTask, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    ref_ptr<Device> device(framebuffer.getDevice());
    auto renderPass = context->renderPass = framebuffer.getRenderPass();
    if (renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(vsg::MultisampleState::create(renderPass->maxSamples));
    context->overridePipelineStates.insert(context->overridePipelineStates.end(), view->overridePipelineStates.begin(), view->overridePipelineStates.end());

    if (view->camera && view->camera->viewportState)
        context->defaultPipelineStates.emplace_back(view->camera->viewportState);
    else
        context->defaultPipelineStates.emplace_back(vsg::ViewportState::create(framebuffer.extent2D()));

    context->view = view.get();
    context->viewID = view->viewID;
    context->viewDependentState = view->viewDependentState;

    contexts.push_back(context);

    if (view->viewDependentState) addViewDependentState(*(view->viewDependentState), device, transferTask, resourceRequirements);
}

void CompileTraversal::add(Framebuffer& framebuffer, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    add(framebuffer, nullptr, view, resourceRequirements);
}

void CompileTraversal::add(const Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    if (viewer.instrumentation) instrumentation = viewer.instrumentation;

    struct AddViews : public Visitor
    {
        CompileTraversal* ct = nullptr;
        const ResourceRequirements& resourceRequirements;
        ref_ptr<TransferTask> transferTask;

        AddViews(CompileTraversal* in_ct, const ResourceRequirements& in_rr) :
            ct(in_ct), resourceRequirements(in_rr){};

        const char* className() const noexcept override { return "vsg::CompileTraversal::AddViews"; }

        std::stack<ref_ptr<Object>> objectStack;

        void apply(Object& object) override
        {
            object.traverse(*this);
        }

        void apply(RenderGraph& rg) override
        {
            if (rg.window)
                objectStack.emplace(rg.window);
            else
                objectStack.emplace(rg.framebuffer);

            rg.traverse(*this);

            objectStack.pop();
        }

        void apply(View& view) override
        {
            if (!objectStack.empty())
            {
                auto obj = objectStack.top();
                if (auto window = obj.cast<Window>())
                    ct->add(*window, transferTask, ref_ptr<View>(&view), resourceRequirements);
                else if (auto framebuffer = obj.cast<Framebuffer>())
                    ct->add(*framebuffer, transferTask, ref_ptr<View>(&view), resourceRequirements);
            }
        }
    } addViews(this, resourceRequirements);

    for (const auto& task : viewer.recordAndSubmitTasks)
    {
        if (resourceRequirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK)
        {
            addViews.transferTask = task->transferTask;
        }

        for (auto& cg : task->commandGraphs)
        {
            cg->accept(addViews);
        }
    }
}

void CompileTraversal::addViewDependentState(ViewDependentState& viewDependentState, ref_ptr<Device> device, ref_ptr<TransferTask> transferTask, const ResourceRequirements& resourceRequirements)
{
    if (viewDependentState.shadowMaps.size() > 0)
    {
        auto context = Context::create(device, resourceRequirements);
        auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
        context->instrumentation = instrumentation;
        context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        context->graphicsQueue = device->getQueue(queueFamily, queueFamilyIndex);
        context->transferTask = transferTask;

        auto& shadowMap = viewDependentState.shadowMaps.front();
        auto& nested_framebuffer = shadowMap.renderGraph->framebuffer;
        if (!nested_framebuffer)
        {
            viewDependentState.compile(*context);
            context->record();
            context->waitForCompletion();
        }

        if (nested_framebuffer)
        {
            add(context, *nested_framebuffer, transferTask, shadowMap.view, resourceRequirements);
        }
        else
        {
            vsg::info("CompileTraversal::addViewDependentState(.., ", device, ", ", transferTask, "..) no frameBufer.");
        }
    }
}

void CompileTraversal::assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation)
{
    instrumentation = in_instrumentation;
    for (const auto& context : contexts)
    {
        context->instrumentation = shareOrDuplicateForThreadSafety(instrumentation);
    }
}

void CompileTraversal::apply(Object& object)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "CompileTraversal Object", COLOR_COMPILE);

    object.traverse(*this);
}

void CompileTraversal::apply(Compilable& node)
{
    CPU_INSTRUMENTATION_L3_NC(instrumentation, "CompileTraversal Compilable", COLOR_COMPILE);

    for (auto& context : contexts)
    {
        node.compile(*context);
    }
}

void CompileTraversal::apply(Commands& commands)
{
    CPU_INSTRUMENTATION_L3_NC(instrumentation, "CompileTraversal Commands", COLOR_COMPILE);

    for (auto& context : contexts)
    {
        commands.compile(*context);
    }
}

void CompileTraversal::apply(Geometry& geometry)
{
    CPU_INSTRUMENTATION_L3_NC(instrumentation, "CompileTraversal Geometry", COLOR_COMPILE);

    for (auto& context : contexts)
    {
        geometry.compile(*context);
    }
    geometry.traverse(*this);
}

void CompileTraversal::apply(CommandGraph& commandGraph)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "CompileTraversal CommandGraph", COLOR_COMPILE);

    for (const auto& context : contexts)
    {
        if (context->resourceRequirements.maxSlot > commandGraph.maxSlot)
        {
            commandGraph.maxSlot = context->resourceRequirements.maxSlot;
        }
    }

    commandGraph.traverse(*this);
}

void CompileTraversal::apply(SecondaryCommandGraph& secondaryCommandGraph)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "CompileTraversal SecondaryCommandGraph", COLOR_COMPILE);

    auto renderPass = secondaryCommandGraph.getRenderPass();

    for (auto& context : contexts)
    {
        if (context->resourceRequirements.maxSlot > secondaryCommandGraph.maxSlot)
        {
            secondaryCommandGraph.maxSlot = context->resourceRequirements.maxSlot;
        }

        // save previous states to be restored after traversal
        auto previousRenderPass = context->renderPass;
        auto previousDefaultPipelineStates = context->defaultPipelineStates;
        auto previousOverridePipelineStates = context->overridePipelineStates;

        context->renderPass = renderPass;

        if (secondaryCommandGraph.window)
        {
            mergeGraphicsPipelineStates(context->mask, context->defaultPipelineStates, ViewportState::create(secondaryCommandGraph.window->extent2D()));
        }
        else if (secondaryCommandGraph.framebuffer)
        {
            mergeGraphicsPipelineStates(context->mask, context->defaultPipelineStates, ViewportState::create(secondaryCommandGraph.framebuffer->extent2D()));
        }

        if (renderPass)
        {
            mergeGraphicsPipelineStates(context->mask, context->overridePipelineStates, MultisampleState::create(context->renderPass->maxSamples));
        }

        secondaryCommandGraph.traverse(*this);

        // restore previous values
        context->defaultPipelineStates = previousDefaultPipelineStates;
        context->overridePipelineStates = previousOverridePipelineStates;
        context->renderPass = previousRenderPass;
    }
}

void CompileTraversal::apply(RenderGraph& renderGraph)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "CompileTraversal RenderGraph", COLOR_COMPILE);

    for (auto& context : contexts)
    {
        // save previous states to be restored after traversal
        auto previousRenderPass = context->renderPass;
        auto previousDefaultPipelineStates = context->defaultPipelineStates;
        auto previousOverridePipelineStates = context->overridePipelineStates;

        mergeGraphicsPipelineStates(context->mask, context->defaultPipelineStates, renderGraph.viewportState);

        context->renderPass = renderGraph.getRenderPass();

        if (context->renderPass)
        {
            mergeGraphicsPipelineStates(context->mask, context->overridePipelineStates, MultisampleState::create(context->renderPass->maxSamples));
        }

        renderGraph.traverse(*this);

        // restore previous values
        context->defaultPipelineStates = previousDefaultPipelineStates;
        context->overridePipelineStates = previousOverridePipelineStates;
        context->renderPass = previousRenderPass;
    }
}

void CompileTraversal::apply(View& view)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "CompileTraversal View", COLOR_COMPILE);

    for (auto& context : contexts)
    {
        // if context is associated with a view make sure we only apply it if it matches with view, otherwise we skip this context
        auto context_view = context->view.ref_ptr();
        if (context_view && context_view.get() != &view) continue;

        // save previous states
        auto previous_view = context->view;
        auto previous_viewID = context->viewID;
        auto previous_mask = context->mask;
        auto previous_overridePipelineStates = context->overridePipelineStates;
        auto previous_defaultPipelineStates = context->defaultPipelineStates;

        context->view = &view;
        context->viewID = view.viewID;
        context->mask = view.mask;
        context->viewDependentState = view.viewDependentState.get();

        if (view.viewDependentState)
        {
            view.viewDependentState->compile(*context);
        }

        // assign view specific pipeline states
        mergeGraphicsPipelineStates(context->mask, context->defaultPipelineStates, view.camera->viewportState);
        mergeGraphicsPipelineStates(context->mask, context->overridePipelineStates, view.overridePipelineStates);

        view.traverse(*this);

        // restore previous states
        context->view = previous_view;
        context->viewID = previous_viewID;
        context->mask = previous_mask;
        context->defaultPipelineStates = previous_defaultPipelineStates;
        context->overridePipelineStates = previous_overridePipelineStates;
    }

    if (view.viewDependentState) view.viewDependentState->accept(*this);
}

bool CompileTraversal::record()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "CompileTraversal record", COLOR_COMPILE);

    bool recorded = false;
    for (auto& context : contexts)
    {
        if (context->record()) recorded = true;
    }
    return recorded;
}

void CompileTraversal::waitForCompletion()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "CompileTraversal waitForCompletion", COLOR_COMPILE);

    for (auto& context : contexts)
    {
        context->waitForCompletion();
    }
}
