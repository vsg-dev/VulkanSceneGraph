/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/CompileManager.h>
#include <vsg/viewer/Viewer.h>
#include <vsg/viewer/View.h>
#include <vsg/io/Options.h>

using namespace vsg;

CompileManager::CompileManager(Viewer& viewer, ref_ptr<ResourceHints> hints)
{
    compileTraversals = CompileTraversals::create(viewer.status);

    ResourceRequirements requirements;

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
    CompileTraversals::container_type  cts;
    while (cts.size() < count)
    {
        auto ct = compileTraversals->take_when_available();
        if (ct) cts.push_back(ct);
        else break;
    }

    return cts;
}

void CompileManager::add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for(auto& ct : cts)
    {
        ct->add(device, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::add(ref_ptr<Window> window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for(auto& ct : cts)
    {
        ct->add(window, viewport, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::add(ref_ptr<Window> window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for(auto& ct : cts)
    {
        ct->add(window, view, resourceRequirements);

        compileTraversals->add(ct);
    }
}

void CompileManager::add(Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    auto cts = takeCompileTraversals(numCompileTraversals);
    for(auto& ct : cts)
    {
        ct->add(viewer, resourceRequirements);

        compileTraversals->add(ct);
    }
}

CompileResult CompileManager::compile(ref_ptr<Object> object)
{
    auto compileTraversal = compileTraversals->take_when_available();

    // if no CompileTraversals are avilable abort compile
    if (!compileTraversal) return {};

    CollectResourceRequirements collectRequirements;
    object->accept(collectRequirements);

    auto& requirements = collectRequirements.requirements;
    auto& binStack = requirements.binStack;

    CompileResult result;
    result.maxSlot = requirements.maxSlot;
    result.containsPagedLOD = requirements.containsPagedLOD;

    for(auto& context : compileTraversal->contexts)
    {
        ref_ptr<View> view = context->view;
        if (view && !binStack.empty())
        {
            auto binDetails = binStack.top();
            result.views[view] = binDetails;
        }

        context->reserve(requirements);
    }

    object->accept(*compileTraversal);

    // std::cout << "Finished compile traversal " << object << std::endl;

    compileTraversal->record(); // records and submits to queue
    compileTraversal->waitForCompletion();

    // std::cout << "Finished waiting for compile " << object << std::endl;

    compileTraversals->add(compileTraversal);

    result.result = VK_SUCCESS;

    // return {};

    return result;
}
