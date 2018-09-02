#pragma once

#include <vsg/vk/DeviceMemory.h>

namespace vsg
{
    class VSG_EXPORT Buffer : public Object
    {
    public:
        Buffer(VkBuffer Buffer, VkBufferUsageFlags usage, VkSharingMode sharingMode, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Buffer, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, AllocationCallbacks* allocator=nullptr);

        VkBufferUsageFlags usage() const { return _usage; }
        VkSharingMode shaderMode() const { return _sharingMode; }
        VkBuffer buffer() const { return _buffer; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        DeviceMemory* getDeviceMemory() { return _deviceMemory; }
        const DeviceMemory* getDeviceMemory() const { return _deviceMemory; }

        VkResult bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset)
        {
            VkResult result = vkBindBufferMemory(*_device, _buffer, *deviceMemory, memoryOffset);
            if (result == VK_SUCCESS)
            {
                _deviceMemory = deviceMemory;
                _memoryOffset = memoryOffset;
            }
            return result;
        }

        operator VkBuffer () const { return _buffer; }

    protected:
        virtual ~Buffer();

        VkBuffer                        _buffer;
        VkBufferUsageFlags              _usage;
        VkSharingMode                   _sharingMode;


        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;

        ref_ptr<DeviceMemory>           _deviceMemory;
        VkDeviceSize                    _memoryOffset;
    };
}
