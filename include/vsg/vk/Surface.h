#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/vk/Instance.h>

namespace vsg
{
    class Surface : public vsg::Object
    {
    public:
        Surface(Instance* instance, VkSurfaceKHR surface, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkSurfaceKHR() const { return _surface; }

    protected:

        virtual ~Surface();

        vsg::ref_ptr<Instance>  _instance;
        VkSurfaceKHR            _surface;
        VkAllocationCallbacks*  _pAllocator;

    };
}
