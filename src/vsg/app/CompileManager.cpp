/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CompileManager.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>

using namespace vsg;

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

CompileResult CompileManager::compile(ref_ptr<Object> object, ContextSelectionFunction contextSelection)
{
    CollectResourceRequirements collectRequirements;
    object->accept(collectRequirements);

    auto& requirements = collectRequirements.requirements;
    auto& binStack = requirements.binStack;

    CompileResult result;
    result.maxSlot = requirements.maxSlot;
    result.containsPagedLOD = requirements.containsPagedLOD;
    result.views = requirements.views;
    result.earlyDynamicData = requirements.earlyDynamicData;
    result.lateDynamicData = requirements.lateDynamicData;

    auto compileTraversal = compileTraversals->take_when_available();

    // if no CompileTraversals are available abort compile
    if (!compileTraversal) return result;

    std::list<ref_ptr<Context>> contexts;
    if (contextSelection)
    {
        for (auto& context : compileTraversal->contexts)
        {
            if (contextSelection(*context)) contexts.push_back(context);
        }

        compileTraversal->contexts.swap(contexts);
    }
    else
    {
        contexts = compileTraversal->contexts;
    }

    for (auto& context : compileTraversal->contexts)
    {
        ref_ptr<View> view = context->view;
        if (view && !binStack.empty())
        {
            if (auto itr = result.views.find(view.get()); itr == result.views.end())
            {
                result.views[view] = binStack.top();
            }
        }

        context->reserve(requirements);
    }

    object->accept(*compileTraversal);

    //debug("Finished compile traversal ", object);

    compileTraversal->record(); // records and submits to queue
    compileTraversal->waitForCompletion();

    debug("Finished waiting for compile ", object);

    compileTraversals->add(compileTraversal);

    // compileTraversal->contexts.swap(contexts);

    result.result = VK_SUCCESS;
    return result;
}
