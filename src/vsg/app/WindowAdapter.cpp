/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/WindowAdapter.h>

using namespace vsg;

WindowAdapter::WindowAdapter(vsg::ref_ptr<vsg::Surface> surface, vsg::ref_ptr<vsg::WindowTraits> traits) :
    Inherit(traits)
{
    if (surface)
    {
        _surface = surface;
        _instance = surface->getInstance();
    }
    if (traits)
    {
        _extent2D.width = traits->width;
        _extent2D.height = traits->height;
    }
}

void WindowAdapter::updateExtents(uint32_t width, uint32_t height)
{
    _extent2D.width = width;
    _extent2D.height = height;
}

void WindowAdapter::resize()
{
    buildSwapchain();
}
