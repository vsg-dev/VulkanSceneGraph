/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Bin.h>

using namespace vsg;

// thread safe container for managing the deviceID for each vsg;:View
static std::mutex s_ViewCountMutex;
static std::vector<bool> s_ActiveViews;

static uint32_t getUniqueViewID()
{
    std::scoped_lock<std::mutex> guard(s_ViewCountMutex);

    uint32_t viewID = 0;
    for (viewID = 0; viewID < static_cast<uint32_t>(s_ActiveViews.size()); ++viewID)
    {
        if (!s_ActiveViews[viewID])
        {
            s_ActiveViews[viewID] = true;
            return viewID;
        }
    }

    s_ActiveViews.push_back(true);

    return viewID;
}

static void releaseViewID(uint32_t viewID)
{
    std::scoped_lock<std::mutex> guard(s_ViewCountMutex);
    s_ActiveViews[viewID] = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// View
//
View::View() :
    viewID(getUniqueViewID()),
    viewDependentState(ViewDependentState::create())
{
}

View::View(ref_ptr<Camera> in_camera, ref_ptr<Node> in_scenegraph) :
    camera(in_camera),
    viewID(getUniqueViewID()),
    viewDependentState(ViewDependentState::create())
{
    if (in_scenegraph) addChild(in_scenegraph);
}

View::~View()
{
    releaseViewID(viewID);
}
