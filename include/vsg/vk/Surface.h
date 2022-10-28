#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>
#include <vsg/vk/Instance.h>

namespace vsg
{
    /// Surface encapsulates VkSurfaceKHR
    class VSG_DECLSPEC Surface : public Inherit<Object, Surface>
    {
    public:
        Surface(VkSurfaceKHR surface, Instance* instance);

        operator VkSurfaceKHR() const { return _surface; }
        VkSurfaceKHR vk() const { return _surface; }

        Instance* getInstance() { return _instance; }
        const Instance* getInstance() const { return _instance; }

        /// Release the VkSurfaceKHR so it's no longer managed this vsg::Surface object, caller/3rd party libs are subsequently responsible for deletion of the VkSurfaceKHR.
        /// This method should only be called when adapting the VulkanSceneGraph to work with 3rd party Vulkan creation,
        /// in normal usage this method should not be called as the VkSurfaceKHR will be cleaned up safely and automatically in the destructor when the vsg::Surface is deleted.
        void release()
        {
            _surface = {};
            _instance = {};
        }

    protected:
        virtual ~Surface();

        VkSurfaceKHR _surface;
        ref_ptr<Instance> _instance;
    };
    VSG_type_name(vsg::Surface);

} // namespace vsg
