#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Window.h>

namespace vsg
{
    /// Class from adapting 3rd party Windowing implementations to a form usable as a vsg::Window
    /// Note, the VulkanSceneGraph provides it's own cross platform Windowing support, WindowAdapter
    /// is only required when 3rd party windowing is used.
    /// The vsgQt project provides an example of WindowAdapter used to adapt Qt window with Vulkan
    /// support for use with VSG applications.
    class VSG_DECLSPEC WindowAdapter : public Inherit<Window, WindowAdapter>
    {
    public:
        WindowAdapter(ref_ptr<Surface> surface, ref_ptr<WindowTraits> traits);

        WindowAdapter() = delete;
        WindowAdapter(const Window&) = delete;
        WindowAdapter& operator=(const Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return nullptr; }

        bool visible() const override { return windowVisible; }
        bool valid() const override { return windowValid; }

        void resize() override;

        /// update the WindowAdapter::_extents and set windowResize to true
        void updateExtents(uint32_t width, uint32_t height);

        // access methods
        VkExtent2D& extent2D() { return _extent2D; }
        bool windowVisible = false;
        bool windowValid = false;

    protected:
        virtual ~WindowAdapter() {}

        void _initSurface() override{};
    };
    VSG_type_name(WindowAdapter);

} // namespace vsg
