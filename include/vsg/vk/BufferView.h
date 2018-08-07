#pragma once

#include <vsg/vk/Buffer.h>

namespace vsg
{
    class BufferView : public Object
    {
    public:
        BufferView(Device* device, VkBufferView bufferView, Buffer* buffer=nullptr, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<BufferView, VkResult, VK_SUCCESS>;

        static Result create(Device* device, Buffer* buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range, AllocationCallbacks* allocator=nullptr);

        operator VkBufferView() const { return _bufferView; }

    protected:

        virtual ~BufferView();

        ref_ptr<Device>                _device;
        VkBufferView                   _bufferView;
        ref_ptr<AllocationCallbacks>   _allocator;
        ref_ptr<Buffer>                _buffer;

    };
}
