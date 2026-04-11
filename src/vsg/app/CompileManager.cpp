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

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CompileResult
//
void CompileResult::reset()
{
    result = VK_INCOMPLETE;
    maxSlots = {};
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

    maxSlots.merge(cr.maxSlots);

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

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CompileManager
//

ResourceScavenger::ResourceScavenger(ref_ptr<DatabasePager> in_databasePager) :
    databasePager(in_databasePager)
{
}

bool ResourceScavenger::scavenge(ResourceRequirements& /*resourceRequirements*/)
{
    bool scavenged = false;

    if (auto ref_databasePager = databasePager.get())
    {
        if (!ref_databasePager->_status->active()) return false;

        uint32_t targetPagedLOD = ref_databasePager->pagedLODContainer->activeList.count;
        if (ref_databasePager->pagedLODContainer->inactiveList.count > ref_databasePager->numActiveRequests) targetPagedLOD += ref_databasePager->pagedLODContainer->inactiveList.count - ref_databasePager->numActiveRequests;

        if (targetPagedLOD < ref_databasePager->targetMaxNumPagedLODWithHighResSubgraphs)
        {
            info("   ResourceScavenger::scavenge(..) resetting, ref_databasePager->targetMaxNumPagedLODWithHighResSubgraphs, to ", targetPagedLOD);

            ref_databasePager->targetMaxNumPagedLODWithHighResSubgraphs = targetPagedLOD;
        }

        auto before_deletedCount = ref_databasePager->_deleteQueue->deletedCount.load();

        if (sleepDuration > 0) std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));

        auto after_deletedCount = ref_databasePager->_deleteQueue->deletedCount.load();

        scavenged = (after_deletedCount > before_deletedCount);
    }

    return scavenged;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CompileManager
//
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

CompileManager::~CompileManager()
{
    vsg::info("CompileManager::~CompileManager() successfulCompileCount= ", successfulCompileCount, ", failedCompileCount = ", failedCompileCount);
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
    CollectResourceRequirements collectRequirements;
    object->accept(collectRequirements);

    auto& requirements = collectRequirements.requirements;
    auto& viewDetailsStack = requirements.viewDetailsStack;

    VkResult reserve_result = VK_INCOMPLETE;
    CompileResult result;
    result.maxSlots = requirements.maxSlots;
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
            }

            for (auto& context : compileTraversal->contexts)
            {
                reserve_result = context->reserve(requirements);

                // vsg::info("  done reserve context->reserve() ",  reserve_result);
                if (reserve_result != VK_SUCCESS && resourceScavenger && resourceScavenger->scavenge(requirements))
                {
                    reserve_result = context->reserve(requirements);
                }

                if (reserve_result != VK_SUCCESS)
                {
                    result.message = vsg::make_string("Context::reserve() failed", reserve_result);
                    result.result = reserve_result;
                    return;
                }
            }

            object->accept(*compileTraversal);

            // if required records and submits to queue
            if (compileTraversal->record())
            {
                compileTraversal->waitForCompletion();
            }
        }
        catch (const vsg::Exception& ve)
        {
            result.message = ve.message;
            result.result = ve.result;
        }
        catch (...)
        {
            result.message = "Exception occurred during compilation.";
            result.result = VK_ERROR_UNKNOWN;
        }

        debug("Finished waiting for compile ", object);
    };

    // assume success, overwrite this on failures.
    result.result = VK_SUCCESS;

    if (contextSelection)
    {
        std::list<ref_ptr<Context>> activeContexts;

        for (auto& context : compileTraversal->contexts)
        {
            if (contextSelection(*context)) activeContexts.push_back(context);
        }

        compileTraversal->contexts.swap(activeContexts);

        run_compile_traversal();

        compileTraversal->contexts.swap(activeContexts);
    }
    else
    {
        run_compile_traversal();
    }

    compileTraversals->add(compileTraversal);

    if (result.result == VK_SUCCESS)
    {
        ++successfulCompileCount;
    }
    else
    {
        ++failedCompileCount;
    }

    return result;
}

CompileResult CompileManager::compileTask(ref_ptr<RecordAndSubmitTask> task, const ResourceRequirements& resourceRequirements)
{
    CompileResult result;

    // assume success, overwrite this on failures.
    result.result = VK_SUCCESS;

    try
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
    }
    catch (const vsg::Exception& ve)
    {
        result.message = ve.message;
        result.result = ve.result;
    }
    catch (...)
    {
        result.message = "Exception occurred during compilation.";
        result.result = VK_ERROR_UNKNOWN;
    }

    return result;
}
