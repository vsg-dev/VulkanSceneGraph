#pragma once

#include <vsg/vk/Surface.h>

namespace vsg
{
    class PhysicalDevice : public vsg::Object
    {
    public:
        PhysicalDevice(Instance* instance, Surface* surface, VkPhysicalDevice device, int gFamily, int pFamily);

        PhysicalDevice(Instance* instance, Surface* surface);

        bool complete() const { return _device!=VK_NULL_HANDLE && _graphicsFamily>=0 && _presentFamily>=0; }

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
