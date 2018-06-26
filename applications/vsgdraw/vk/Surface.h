#pragma once

#include "vk/Instance.h"

#include <vsg/core/ref_ptr.h>

#include <set>
#include <iostream>

/////////////////////////////////////////////////////////////////////
//
// start of vulkan code
//
namespace vsg
{
    class Surface : public vsg::Object
    {
    public:
        Surface(Instance* instance, VkSurfaceKHR surface, VkAllocationCallbacks* pAllocator=nullptr) : _instance(instance), _surface(surface), _pAllocator(pAllocator) {}

        operator VkSurfaceKHR() const { return _surface; }

    protected:
        virtual ~Surface()
        {
            if (_surface)
            {
                std::cout<<"Calling vkDestroySurfaceKHR"<<std::endl;
                vkDestroySurfaceKHR(*_instance, _surface, _pAllocator);
            }
        }

        vsg::ref_ptr<Instance>  _instance;
        VkSurfaceKHR            _surface;
        VkAllocationCallbacks*  _pAllocator;

    };
}
