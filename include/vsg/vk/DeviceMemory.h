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

    class VSG_DECLSPEC MemorySlots
    {
    public:
        MemorySlots(VkDeviceSize availableMemorySize);

        using OptionalOffset = std::pair<bool, VkDeviceSize>;
        OptionalOffset reserve(VkDeviceSize size, VkDeviceSize alignment);

        void release(VkDeviceSize offset, VkDeviceSize size);

        bool full() const { return _availableMemory.empty(); }

        void report() const;
        bool check() const;

    protected:
        using SizeOffsets = std::multimap<VkDeviceSize, VkDeviceSize>;
        using SizeOffset = SizeOffsets::value_type;
        SizeOffsets _availableMemory;

        using OffsetSizes = std::map<VkDeviceSize, VkDeviceSize>;
        using OffsetSize = OffsetSizes::value_type;
        OffsetSizes _offsetSizes;

        using OffsetAllocatedSlot = std::map<VkDeviceSize, OffsetSize>;
        OffsetSizes _reservedOffsetSizes;

        VkDeviceSize _totalMemorySize;
    };

    class VSG_DECLSPEC DeviceMemory : public Inherit<Object, DeviceMemory>
    {
    public:
        DeviceMemory(VkDeviceMemory DeviceMemory, const VkMemoryRequirements& memRequirements, Device* device, AllocationCallbacks* allocator = nullptr);

        using Result = vsg::Result<DeviceMemory, VkResult, VK_SUCCESS>;
        static Result create(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, void* pNextAllocInfo = nullptr, AllocationCallbacks* allocator = nullptr);

        static Result create(Device* device, Buffer* buffer, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator = nullptr);
        static Result create(Device* device, Image* image, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator = nullptr);

        void copy(VkDeviceSize offset, VkDeviceSize size, const void* src_data);
        void copy(VkDeviceSize offset, const Data* data);

        /// wrapper of vkMapMemory
        VkResult map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
        void unmap();

        operator VkDeviceMemory() const { return _deviceMemory; }

        const VkMemoryRequirements& getMemoryRequirements() { return _memoryRequirements; }

        MemorySlots::OptionalOffset reserve(VkDeviceSize size) { return _memorySlots.reserve(size, _memoryRequirements.alignment); }
        void release(VkDeviceSize offset, VkDeviceSize size) { _memorySlots.release(offset, size); }
        bool full() const { return _memorySlots.full(); }

    protected:
        virtual ~DeviceMemory();

        VkDeviceMemory _deviceMemory;
        VkMemoryRequirements _memoryRequirements;
        ref_ptr<Device> _device;
        ref_ptr<AllocationCallbacks> _allocator;

        MemorySlots _memorySlots;
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

        template<typename... Args>
        static ref_ptr<MappedData> create(Args... args)
        {
            return ref_ptr<MappedData>(new MappedData(args...));
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
