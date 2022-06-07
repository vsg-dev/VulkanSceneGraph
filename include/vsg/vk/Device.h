#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DeviceFeatures.h>
#include <vsg/vk/Extensions.h>
#include <vsg/vk/Queue.h>

#include <list>

namespace vsg
{

    // forward declare
    class WindowTraits;

    struct QueueSetting
    {
        int queueFamilyIndex = -1;
        std::vector<float> queuePiorities;
    };

    using QueueSettings = std::vector<QueueSetting>;

    class VSG_DECLSPEC Device : public Inherit<Object, Device>
    {
    public:
        Device() = default;
        Device(PhysicalDevice* physicalDevice, const QueueSettings& queueSettings, const Names& layers, const Names& deviceExtensions, const DeviceFeatures* deviceFeatures = nullptr, AllocationCallbacks* allocator = nullptr);

        Instance* getInstance() { return _instance.get(); }
        const Instance* getInstance() const { return _instance.get(); }

        PhysicalDevice* getPhysicalDevice() { return _physicalDevice.get(); }
        const PhysicalDevice* getPhysicalDevice() const { return _physicalDevice.get(); }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

        const Extensions* getExtensions() const { return _extensions.get(); }

        operator VkDevice() const { return _device; }
        VkDevice getDevice() const { return _device; }

        static uint32_t maxNumDevices();

        const uint32_t deviceID = 0;

        ref_ptr<Queue> getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex = 0);

    protected:
        virtual ~Device();

        VkDevice _device;

        ref_ptr<Instance> _instance;
        ref_ptr<PhysicalDevice> _physicalDevice;
        ref_ptr<AllocationCallbacks> _allocator;
        ref_ptr<Extensions> _extensions;

        std::list<ref_ptr<Queue>> _queues;
    };
    VSG_type_name(vsg::Device);

} // namespace vsg
