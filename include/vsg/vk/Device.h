#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/PhysicalDevice.h>

namespace vsg
{
    class VSG_DECLSPEC Device : public Inherit<Object, Device>
    {
    public:
        Device(VkDevice device, PhysicalDevice* physicalDevice, AllocationCallbacks* allocator = nullptr);

        using Result = vsg::Result<Device, VkResult, VK_SUCCESS>;
        static Result create(PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator = nullptr);

        const PhysicalDevice* getPhysicalDevice() const { return _physicalDevice.get(); }

        operator VkDevice() const { return _device; }
        VkDevice getDevice() const { return _device; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

        VkQueue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex = 0);

    protected:
        virtual ~Device();

        VkDevice _device;
        vsg::ref_ptr<PhysicalDevice> _physicalDevice;
        vsg::ref_ptr<AllocationCallbacks> _allocator;
    };

} // namespace vsg
