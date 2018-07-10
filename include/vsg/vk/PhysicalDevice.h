#pragma once

#include <vsg/vk/Surface.h>

namespace vsg
{
    class PhysicalDevice : public vsg::Object
    {
    public:
        PhysicalDevice(Instance* instance, Surface* surface, VkPhysicalDevice device, int gFamily, int pFamily);

        using Result = vsg::Result<PhysicalDevice, VkResult, VK_SUCCESS>;
        static Result create(Instance* instance, Surface* surface);

        bool complete() const { return _device!=VK_NULL_HANDLE && _graphicsFamily>=0 && _presentFamily>=0; }

        const Instance* getInstance() const { return _instance.get(); }
        const Surface* getSurface() const { return _surface.get(); }

        operator VkPhysicalDevice() const { return _device; }
        VkPhysicalDevice getPhysicalDevice() const { return _device; }

        int getGraphicsFamily() const { return _graphicsFamily; }
        int getPresentFamily() const { return _presentFamily; }

    protected:

        virtual ~PhysicalDevice();

        vsg::ref_ptr<Instance>  _instance;
        vsg::ref_ptr<Surface>   _surface;

        VkPhysicalDevice _device;
        int _graphicsFamily;
        int _presentFamily;
    };


}
