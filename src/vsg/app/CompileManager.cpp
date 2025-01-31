/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CompileManager.h>
#include <vsg/app/RecordAndSubmitTask.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/utils/ShaderSet.h>

using namespace vsg;

void CompileResult::reset()
{
    result = VK_INCOMPLETE;
    maxSlot = 0;
    containsPagedLOD = false;
    views.clear();
    dynamicData.clear();
}

void CompileResult::add(const CompileResult& cr)
{
    if (result == VK_INCOMPLETE || result == VK_SUCCESS)
    {
        result = cr.result;
    }

    if (cr.maxSlot > maxSlot) maxSlot = cr.maxSlot;
    if (!containsPagedLOD) containsPagedLOD = cr.containsPagedLOD;

    for (auto& [src_view, src_binDetails] : cr.views)
    {
        if (src_binDetails.indices.empty() && src_binDetails.bins.empty()) break;

        auto& binDetails = views[src_view];
        binDetails.indices.insert(src_binDetails.indices.begin(), src_binDetails.indices.end());
        binDetails.bins.insert(src_binDetails.bins.begin(), src_binDetails.bins.end());
    }

    dynamicData.add(dynamicData);
}

bool CompileResult::requiresViewerUpdate() const
{
    if (result == VK_INCOMPLETE) return false;

    if (dynamicData) return true;

    for (auto& [view, binDetails] : views)
    {
        if (!binDetails.indices.empty() || !binDetails.bins.empty()) return true;
    }
    return false;
}

CompileManager::CompileManager(Viewer& viewer, ref_ptr<ResourceHints> hints)
{
    compileTraversals = CompileTraversals::create(viewer.status);

    ResourceRequirements requirements(hints);

    auto ct = CompileTraversal::create(viewer, requirements);
    compileTraversals->add(ct);
#if 0
    compileTraversals->add(CompileTraversal::create(*ct));
    compileTraversals->add(CompileTraversal::create(*ct));
    compileTraversals->add(CompileTraversal::create(*ct));
    numCompileTraversals = 4;
#else
    numCompileTraversals = 1;
#endif
}

CompileManager::CompileTraversals::container_type CompileManager::takeCompileTraversals(size_t count)
{
    CompileTraversals::container_type cts;
    while (cts.size() < count)
    {
        auto ct = compileTraversals->take_when_available();
        if (ct)
            cts.push_back(ct);
        else
            break;
    }

    return cts;
}

void CompileManager::add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for (auto& ct : cts)
    {
        ct->add(device, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::add(Window& window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for (auto& ct : cts)
    {
        ct->add(window, viewport, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::add(Window& window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for (auto& ct : cts)
    {
        ct->add(window, view, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::add(Framebuffer& framebuffer, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for (auto& ct : cts)
    {
        ct->add(framebuffer, view, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::add(const Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for (auto& ct : cts)
    {
        ct->add(viewer, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for (auto& ct : cts)
    {
        ct->assignInstrumentation(in_instrumentation);

        compileTraversals->add(ct);
    }
}

CompileResult CompileManager::compile(ref_ptr<Object> object, ContextSelectionFunction contextSelection)
{
    vsg::debug("CompileManager::compile(", object, ", ..)");

    CollectResourceRequirements collectRequirements;
    object->accept(collectRequirements);

    auto& requirements = collectRequirements.requirements;
    auto& viewDetailsStack = requirements.viewDetailsStack;

    CompileResult result;
    result.maxSlot = requirements.maxSlot;
    result.containsPagedLOD = requirements.containsPagedLOD;
    result.views = requirements.views;
    result.dynamicData = requirements.dynamicData;

    auto compileTraversal = compileTraversals->take_when_available();

    // if no CompileTraversals are available abort compile
    if (!compileTraversal) return result;

    auto run_compile_traversal = [&]() -> void {
        try
        {
            for (auto& context : compileTraversal->contexts)
            {
                ref_ptr<View> view = context->view;

                if (view)
                {
                    result.views[view].add(viewDetailsStack.top());
                    if (view->viewDependentState)
                    {
                        for (auto& sm : view->viewDependentState->shadowMaps)
                        {
                            if (sm.view)
                            {
                                result.views[sm.view].add(requirements.viewDetailsStack.top());
                            }
                        }
                    }
                }
                context->reserve(requirements);
            }

            object->accept(*compileTraversal);

            //debug("Finished compile traversal ", object);

            // if required records and submits to queue
            if (compileTraversal->record())
            {
                compileTraversal->waitForCompletion();
            }
        }
        catch (const vsg::Exception& ve)
        {
            vsg::debug("CompileManager::compile() exception caught : ", ve.message);
            result.message = ve.message;
            result.result = ve.result;
        }
        catch (...)
        {
            vsg::debug("CompileManager::compile() exception caught");
            result.message = "Exception occurred during compilation.";
            result.result = VK_ERROR_UNKNOWN;
        }

        debug("Finished waiting for compile ", object);
    };

    // assume success, overite this on failures.
    result.result = VK_SUCCESS;

    if (contextSelection)
    {
        std::list<ref_ptr<Context>> contexts;

        for (auto& context : compileTraversal->contexts)
        {
            if (contextSelection(*context)) contexts.push_back(context);
        }

        compileTraversal->contexts.swap(contexts);

        run_compile_traversal();

        compileTraversal->contexts.swap(contexts);
    }
    else
    {
        run_compile_traversal();
    }

    compileTraversals->add(compileTraversal);

    return result;
}

CompileResult CompileManager::compileTask(ref_ptr<RecordAndSubmitTask> task, const ResourceRequirements& resourceRequirements)
{
    auto compileTraversal = CompileTraversal::create(task->device, resourceRequirements);

    for (const auto& context : compileTraversal->contexts)
    {
        if (resourceRequirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK)
        {
            context->transferTask = task->transferTask;
        }
    }

    for (auto& cg : task->commandGraphs)
    {
        cg->accept(*compileTraversal);
    }

    if (compileTraversal->record())
    {
        compileTraversal->waitForCompletion();
    }

    return {};
}
