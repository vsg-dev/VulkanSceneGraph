#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>

namespace vsg
{
    class VSG_EXPORT MemoryManager : public Inherit<Object, MemoryManager>
    {
    public:
        MemoryManager(Device* device, AllocationCallbacks* allocator=nullptr);

        Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);

        DeviceMemory* createMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties);

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~MemoryManager();

        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
