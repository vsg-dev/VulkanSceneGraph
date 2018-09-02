#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/vk/Instance.h>

namespace vsg
{
    class VSG_EXPORT Surface : public vsg::Object
    {
    public:
        Surface(VkSurfaceKHR surface, Instance* instance, AllocationCallbacks* allocator=nullptr);

        operator VkSurfaceKHR() const { return _surface; }

        Instance* getInstance() { return _instance; }
        const Instance* getInstance() const { return _instance; }


    protected:

        virtual ~Surface();

        VkSurfaceKHR                    _surface;
        ref_ptr<Instance>               _instance;
        ref_ptr<AllocationCallbacks>    _allocator;

    };
}
