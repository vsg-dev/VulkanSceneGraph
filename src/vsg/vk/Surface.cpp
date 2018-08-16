#include <vsg/vk/Surface.h>

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
        vkDestroySurfaceKHR(*_instance, _surface, _allocator);
    }
}

}
