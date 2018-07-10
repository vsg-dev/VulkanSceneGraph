#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/vk/Instance.h>

namespace vsg
{
    class Surface : public vsg::Object
    {
    public:
        Surface(Instance* instance, VkSurfaceKHR surface, AllocationCallbacks* allocator=nullptr);

        operator VkSurfaceKHR() const { return _surface; }

    protected:

        virtual ~Surface();

        ref_ptr<Instance>               _instance;
        VkSurfaceKHR                    _surface;
        ref_ptr<AllocationCallbacks>    _allocator;

    };
}
