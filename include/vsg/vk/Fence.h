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
    /// Fence encapsulates vkFence
    /// Used for synchronizing the completion of Vulkan command submissions to queues
    class VSG_DECLSPEC Fence : public Inherit<Object, Fence>
    {
    public:
        explicit Fence(Device* device, VkFenceCreateFlags flags = 0);

        operator VkFence() const { return _vkFence; }
        VkFence vk() const { return _vkFence; }

        VkResult wait(uint64_t timeout);

        VkResult reset() const;

        VkResult status() const { return vkGetFenceStatus(*_device, _vkFence); }
        bool hasDependencies() const
        {
            return !(_dependentSemaphores.empty()
                     && _dependentCommandBuffers.empty()
                     && _actions.empty());
        }

        void resetFenceAndDependencies();

        Semaphores& dependentSemaphores() { return _dependentSemaphores; }
        CommandBuffers& dependentCommandBuffers() { return _dependentCommandBuffers; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }
        using action_container_type = std::list<FenceAction>;
        void putActions(uint64_t in_submission, action_container_type& actions);
        uint64_t submission;
        action_container_type& actions() { return _actions; }
    protected:
        virtual ~Fence();

        VkFence _vkFence;
        Semaphores _dependentSemaphores;
        CommandBuffers _dependentCommandBuffers;

        ref_ptr<Device> _device;
        action_container_type _actions;
    };
    VSG_type_name(vsg::Fence);

} // namespace vsg
