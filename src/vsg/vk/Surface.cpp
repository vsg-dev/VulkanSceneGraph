#include <vsg/vk/Surface.h>

#include <iostream>

namespace vsg
{

Surface::Surface(Instance* instance, VkSurfaceKHR surface, VkAllocationCallbacks* pAllocator) :
    _instance(instance),
    _surface(surface),
    _pAllocator(pAllocator)
{
}

Surface::~Surface()
{
    if (_surface)
    {
        std::cout<<"Calling vkDestroySurfaceKHR"<<std::endl;
        vkDestroySurfaceKHR(*_instance, _surface, _pAllocator);
    }
}

}
