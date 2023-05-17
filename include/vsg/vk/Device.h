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

    /// Device encapsulate vkDeivce, a logical handle to the PhysicalDevice with capabilities specified during construction.
    class VSG_DECLSPEC Device : public Inherit<Object, Device>
    {
    public:
        Device(PhysicalDevice* physicalDevice, const QueueSettings& queueSettings, Names layers, Names deviceExtensions, const DeviceFeatures* deviceFeatures = nullptr, AllocationCallbacks* allocator = nullptr);

        operator VkDevice() const { return _device; }
        VkDevice vk() const { return _device; }

        static uint32_t maxNumDevices();
        const uint32_t deviceID = 0;

        Instance* getInstance() { return _instance.get(); }
        const Instance* getInstance() const { return _instance.get(); }

        PhysicalDevice* getPhysicalDevice() { return _physicalDevice.get(); }
        const PhysicalDevice* getPhysicalDevice() const { return _physicalDevice.get(); }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

        ref_ptr<Queue> getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex = 0);

        const Extensions* getExtensions() const { return _extensions.get(); }

        /// get the address of specified function using vkGetDeviceProcAddr
        /// for core commands beyond the apiVersion specified in vsg::Instance creation, vkGetDeviceProcAddr may return a non-nullptr function pointer, though the function pointer must not be called.
        /// for extension commands, vkGetDeviceProcAddr will always return nullptr if the extension is not enabled in vsg::Device creation.
        template<typename T>
        bool getProcAddr(T& procAddress, const char* pName, const char* pNameFallback = nullptr) const
        {
            procAddress = reinterpret_cast<T>(vkGetDeviceProcAddr(_device, pName));
            if (procAddress) return true;

            if (pNameFallback) procAddress = reinterpret_cast<T>(vkGetDeviceProcAddr(_device, pNameFallback));
            return (procAddress);
        }

        /// device-level core functionality can be used if both VkInstance and VkPhysicalDevice support the Vulkan version that provides it.
        bool supportsApiVersion(uint32_t version) const;

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
