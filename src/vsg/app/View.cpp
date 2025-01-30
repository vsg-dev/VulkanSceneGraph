/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/nodes/Bin.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/Context.h>

using namespace vsg;

// thread safe container for managing the deviceID for each vsg::View
static std::mutex s_ViewCountMutex;
static std::vector<uint32_t> s_ActiveViews;

static uint32_t getUniqueViewID()
{
    std::scoped_lock<std::mutex> guard(s_ViewCountMutex);

    uint32_t viewID = 0;
    for (viewID = 0; viewID < static_cast<uint32_t>(s_ActiveViews.size()); ++viewID)
    {
        if (s_ActiveViews[viewID] == 0)
        {
            ++s_ActiveViews[viewID];
            return viewID;
        }
    }

    s_ActiveViews.push_back(1);

    return viewID;
}

static uint32_t sharedViewID(uint32_t viewID)
{
    std::scoped_lock<std::mutex> guard(s_ViewCountMutex);

    if (viewID < static_cast<uint32_t>(s_ActiveViews.size()))
    {
        ++s_ActiveViews[viewID];
        return viewID;
    }

    viewID = static_cast<uint32_t>(s_ActiveViews.size());
    s_ActiveViews.push_back(1);

    return viewID;
}

static void releaseViewID(uint32_t viewID)
{
    std::scoped_lock<std::mutex> guard(s_ViewCountMutex);
    --s_ActiveViews[viewID];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// View
//
View::View(ViewFeatures in_features) :
    viewID(getUniqueViewID()),
    features(in_features)
{
    viewDependentState = ViewDependentState::create(this);
}

View::View(const View& view) :
    Inherit(view),
    viewID(sharedViewID(view.viewID)),
    features(view.features),
    mask(view.mask)
{
    if (view.camera && view.camera->viewportState)
    {
        camera = vsg::Camera::create();
        camera->viewportState = view.camera->viewportState;
    }

    viewDependentState = ViewDependentState::create(this);

    // info("View::View(const View&) ", this, ", ", viewDependentState, ", ", viewID);
}

View::View(ref_ptr<Camera> in_camera, ref_ptr<Node> in_scenegraph, ViewFeatures in_features) :
    camera(in_camera),
    viewID(getUniqueViewID()),
    features(in_features)
{
    if (in_scenegraph) addChild(in_scenegraph);

    viewDependentState = ViewDependentState::create(this);

    // info("View::View(ref_ptr<Camera> in_camera) ", this, ", ", viewDependentState, ", ", viewID);
}

View::~View()
{
    if (viewDependentState) viewDependentState->view = nullptr;
    releaseViewID(viewID);
}

void View::share(const View& view)
{
    if (viewID != view.viewID)
    {
        releaseViewID(viewID);
        const_cast<uint32_t&>(viewID) = sharedViewID(view.viewID);
    }

    mask = view.mask;
    if (view.camera && view.camera->viewportState)
    {
        if (!camera) camera = vsg::Camera::create();
        camera->viewportState = view.camera->viewportState;
    }
}
