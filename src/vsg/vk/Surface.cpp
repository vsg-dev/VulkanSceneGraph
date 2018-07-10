#include <vsg/vk/Surface.h>

#include <iostream>

namespace vsg
{

Surface::Surface(Instance* instance, VkSurfaceKHR surface, AllocationCallbacks* allocator) :
    _instance(instance),
    _surface(surface),
    _allocator(allocator)
{
}

Surface::~Surface()
{
    if (_surface)
    {
        std::cout<<"Calling vkDestroySurfaceKHR"<<std::endl;
        vkDestroySurfaceKHR(*_instance, _surface, *_allocator);
    }
}

}
