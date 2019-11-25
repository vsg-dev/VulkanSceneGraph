#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Surface.h>

namespace vsg
{
    class VSG_DECLSPEC PhysicalDevice : public Inherit<Object, PhysicalDevice>
    {
    public:
        PhysicalDevice(Instance* instance, VkPhysicalDevice device, int graphicsFamily, int presentFamily, int computeFamily, Surface* surface);

        using Result = vsg::Result<PhysicalDevice, VkResult, VK_SUCCESS>;
        static Result create(Instance* instance, VkQueueFlags queueFlags, Surface* surface = nullptr);

        bool complete() const { return _device != VK_NULL_HANDLE && _graphicsFamily >= 0 && _presentFamily >= 0; }

        const Instance* getInstance() const { return _instance.get(); }
        const Surface* getSurface() const { return _surface.get(); }

        operator VkPhysicalDevice() const { return _device; }
        VkPhysicalDevice getPhysicalDevice() const { return _device; }

        int getGraphicsFamily() const { return _graphicsFamily; }
        int getPresentFamily() const { return _presentFamily; }
        int getComputeFamily() const { return _computeFamily; }

        const VkPhysicalDeviceProperties& getProperties() const { return _properties; }
        const VkPhysicalDeviceRayTracingPropertiesNV& getRayTracingProperties() const { return _rayTracingProperties; }

    protected:
        virtual ~PhysicalDevice();

        VkPhysicalDevice _device;
        int _graphicsFamily;
        int _presentFamily;
        int _computeFamily;

        VkPhysicalDeviceProperties _properties;
        VkPhysicalDeviceRayTracingPropertiesNV _rayTracingProperties;

        vsg::ref_ptr<Instance> _instance;
        vsg::ref_ptr<Surface> _surface;
    };

} // namespace vsg
