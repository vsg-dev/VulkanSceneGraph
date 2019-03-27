#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/vk/Device.h>

#include <map>

namespace vsg
{
    class Buffer;
    class Image;

    class VSG_DECLSPEC DeviceMemory : public Inherit<Object, DeviceMemory>
    {
    public:
        DeviceMemory(VkDeviceMemory DeviceMemory, const VkMemoryRequirements& memRequirements, Device* device, AllocationCallbacks* allocator = nullptr);

        using Result = vsg::Result<DeviceMemory, VkResult, VK_SUCCESS>;
        static Result create(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator = nullptr);

        static Result create(Device* device, Buffer* buffer, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator = nullptr);
        static Result create(Device* device, Image* image, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator = nullptr);

        void copy(VkDeviceSize offset, VkDeviceSize size, const void* src_data);
        void copy(VkDeviceSize offset, const Data* data);

        /// wrapper of vkMapMemory
        VkResult map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
        void unmap();

        operator VkDeviceMemory() const { return _deviceMemory; }

        const VkMemoryRequirements& getMemoryRequirements() { return _memoryRequirements; }

        using OptionalMemoryOffset = std::pair<bool, VkDeviceSize>;
        OptionalMemoryOffset reserve(VkDeviceSize size);

        bool full() const { return _availableMemory.empty(); }

    protected:
        virtual ~DeviceMemory();

        VkDeviceMemory _deviceMemory;
        VkMemoryRequirements _memoryRequirements;
        ref_ptr<Device> _device;
        ref_ptr<AllocationCallbacks> _allocator;

        using MemorySlots = std::multimap<VkDeviceSize, VkDeviceSize>;
        using MemorySlot = MemorySlots::value_type;
        MemorySlots _availableMemory;

    };

    template<class T>
    class MappedData : public T
    {
    public:
        using value_type = typename T::value_type;

        template<typename... Args>
        MappedData(DeviceMemory* deviceMemory, VkDeviceSize offset, VkMemoryMapFlags flags, Args&... args) :
            T(),
            _deviceMemory(deviceMemory)
        {
            void* pData;
            size_t numElements = (args * ...);
            _deviceMemory->map(offset, numElements * sizeof(value_type), flags, &pData);
            T::assign(args..., static_cast<value_type*>(pData));
        }

        virtual ~MappedData()
        {
            T::dataRelease(); // make sure that the Array doesn't delete this memory
            _deviceMemory->unmap();
        }

    protected:
        ref_ptr<DeviceMemory> _deviceMemory;
    };

} // namespace vsg
