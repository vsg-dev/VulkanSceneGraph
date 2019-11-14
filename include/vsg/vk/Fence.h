#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Semaphore.h>

namespace vsg
{
    class VSG_DECLSPEC Fence : public Inherit<Object, Fence>
    {
    public:
        Fence(VkFence Fence, Device* device, AllocationCallbacks* allocator = nullptr);

        using Result = vsg::Result<Fence, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkFenceCreateFlags flags = 0, AllocationCallbacks* allocator = nullptr);

        VkResult wait(uint64_t timeout) const { return vkWaitForFences(*_device, 1, &_vkFence, VK_TRUE, timeout); }

        VkResult reset() const { return vkResetFences(*_device, 1, &_vkFence); }

        VkResult status() const { return vkGetFenceStatus(*_device, _vkFence); }

        VkFence fence() const { return _vkFence; }

        operator VkFence() const { return _vkFence; }

        Semaphores& dependentSemaphores() { return _dependentSemaphores; }
        CommandBuffers& dependentCommandBuffers() { return _dependentCommandBuffers; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~Fence();

        VkFence _vkFence;
        Semaphores _dependentSemaphores;
        CommandBuffers _dependentCommandBuffers;

        ref_ptr<Device> _device;
        ref_ptr<AllocationCallbacks> _allocator;
    };

} // namespace vsg
