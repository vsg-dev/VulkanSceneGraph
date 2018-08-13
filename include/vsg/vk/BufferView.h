#pragma once

#include <vsg/vk/Buffer.h>

namespace vsg
{
    class BufferView : public Object
    {
    public:
        BufferView(VkBufferView bufferView, Device* device, Buffer* buffer=nullptr, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<BufferView, VkResult, VK_SUCCESS>;

        static Result create(Buffer* buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range, AllocationCallbacks* allocator=nullptr);

        operator VkBufferView() const { return _bufferView; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        Buffer* getBuffer() { return _buffer; }
        const Buffer* getBuffer() const { return _buffer; }

    protected:

        virtual ~BufferView();

        VkBufferView                   _bufferView;
        ref_ptr<Device>                _device;
        ref_ptr<Buffer>                _buffer;
        ref_ptr<AllocationCallbacks>   _allocator;
    };
}
