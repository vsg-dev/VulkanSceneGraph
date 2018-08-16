#include <vsg/vk/Surface.h>

#include <iostream>

namespace vsg
{

Surface::Surface(VkSurfaceKHR surface, Instance* instance, AllocationCallbacks* allocator) :
    _surface(surface),
    _instance(instance),
    _allocator(allocator)
{
}

Surface::~Surface()
{
    if (_surface)
    {
        std::cout<<"Calling vkDestroySurfaceKHR"<<std::endl;
        vkDestroySurfaceKHR(*_instance, _surface, _allocator);
    }
}

}
