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

    /// DeviceMemory encapsulates vkDeviceMemory.
    /// DeviceMemory maps to memory on the CPU or GPU depending on the properties that it's set up with.
    class VSG_DECLSPEC DeviceMemory : public Inherit<Object, DeviceMemory>
    {
    public:
        DeviceMemory(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, void* pNextAllocInfo = nullptr);

        operator VkDeviceMemory() const { return _deviceMemory; }
        VkDeviceMemory vk() const { return _deviceMemory; }

        void copy(VkDeviceSize offset, VkDeviceSize size, const void* src_data);
        void copy(VkDeviceSize offset, const Data* data);

        /// wrapper of vkMapMemory
        VkResult map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
        void unmap();

        const VkMemoryRequirements& getMemoryRequirements() const { return _memoryRequirements; }
        const VkMemoryPropertyFlags& getMemoryPropertyFlags() const { return _properties; }

        MemorySlots::OptionalOffset reserve(VkDeviceSize size);
        void release(VkDeviceSize offset, VkDeviceSize size);

        bool full() const;
        VkDeviceSize maximumAvailableSpace() const;
        size_t totalAvailableSize() const;
        size_t totalReservedSize() const;

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~DeviceMemory();

        VkDeviceMemory _deviceMemory;
        VkMemoryRequirements _memoryRequirements;
        VkMemoryPropertyFlags _properties;
        ref_ptr<Device> _device;

        mutable std::mutex _mutex;
        MemorySlots _memorySlots;
    };
    VSG_type_name(vsg::DeviceMemory);

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
        static ref_ptr<MappedData> create(DeviceMemory* deviceMemory, VkDeviceSize offset, VkMemoryMapFlags flags, Args... args)
        {
            return ref_ptr<MappedData>(new MappedData(deviceMemory, offset, flags, args...));
        }

        template<typename... Args>
        static ref_ptr<MappedData> create(DeviceMemory* deviceMemory, VkDeviceSize offset, VkMemoryMapFlags flags, Data::Properties properties, Args... args)
        {
            auto data = ref_ptr<MappedData>(new MappedData(deviceMemory, offset, flags, args...));
            data->properties = properties;
            return data;
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
