#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/vk/Instance.h>

namespace vsg
{
    class Surface : public vsg::Object
    {
    public:
        Surface(VkSurfaceKHR surface, Instance* instance, AllocationCallbacks* allocator=nullptr);

        operator VkSurfaceKHR() const { return _surface; }

    protected:

        virtual ~Surface();

        VkSurfaceKHR                    _surface;
        ref_ptr<Instance>               _instance;
        ref_ptr<AllocationCallbacks>    _allocator;

    };
}
