#pragma once

#include <vsg/core/Array.h>
#include <vsg/vk/Device.h>

namespace vsg
{
    class Buffer;
    class Image;

    class DeviceMemory : public Object
    {
    public:
        DeviceMemory(VkDeviceMemory DeviceMemory, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<DeviceMemory, VkResult, VK_SUCCESS>;
        static Result create(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator=nullptr);

        static Result create(Device* device, Buffer* buffer, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator=nullptr);
        static Result create(Device* device, Image* image, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator=nullptr);

        void copy(VkDeviceSize offset, VkDeviceSize size, void* src_data);
        void copy(VkDeviceSize offset, Data* data);

        /// wrapper of vkMapMemory
        VkResult map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
        void unmap();

        operator VkDeviceMemory () const { return _deviceMemory; }

    protected:
        virtual ~DeviceMemory();

        VkDeviceMemory                  _deviceMemory;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

    template<class T>
    class MappedArray : public T
    {
    public:

        using value_type = typename T::value_type;

        MappedArray(DeviceMemory* deviceMemory, VkDeviceSize offset, size_t numElements, VkMemoryMapFlags flags=0) :
            T(),
            _deviceMemory(deviceMemory)
        {
            void* pData;
            _deviceMemory->map(offset, numElements*sizeof(value_type), flags, &pData);
            T::assign(numElements, static_cast<value_type*>(pData));
        }

        virtual ~MappedArray()
        {
            T::dataRelease(); // make sure that the Array doesn't delete this memory
            _deviceMemory->unmap();
        }
    protected:
        ref_ptr<DeviceMemory> _deviceMemory;
    };


}
