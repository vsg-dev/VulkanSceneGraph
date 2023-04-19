#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>
#include <vsg/vk/vulkan.h>

#include <functional>
#include <mutex>
#include <list>
#include <vector>

namespace vsg
{
    // forward declare
    class Fence;

    //// An action that will be performed once a Fence is waited on successfully.
    using FenceAction = std::pair<uint64_t, std::function<void()>>;
    
    /// Queue encapsulates a single vkQueue, used to submit vulkan commands for processing.
    class VSG_DECLSPEC Queue : public Inherit<Object, Queue>
    {
    public:
        operator VkQueue() const { return _vkQueue; }
        VkQueue vk() const { return _vkQueue; }

        uint32_t queueFamilyIndex() const { return _queueFamilyIndex; }
        uint32_t queueIndex() const { return _queueIndex; }

        VkResult submit(const std::vector<VkSubmitInfo>& submitInfos, Fence* fence = nullptr);

        VkResult submit(const VkSubmitInfo& submitInfo, Fence* fence = nullptr);

        //// Queue submit with a functor that will be called when the commands in this submission
        //// have completed. This happens when a fence passed in a submission -- either this or a
        //// later one -- is waited upon. Vulkan guarantees that the commands will have finished at
        //// that point.
        VkResult submit(const std::vector<VkSubmitInfo>& submitInfos, std::function<void()> func,
                        Fence* fence = nullptr);
        VkResult submit(const VkSubmitInfo& submitInfo, std::function<void()> func, Fence* fence = nullptr);

        VkResult present(const VkPresentInfoKHR& info);

        VkResult waitIdle();

    protected:
        Queue(VkQueue queue, uint32_t queueFamilyIndex, uint32_t queueIndex);
        virtual ~Queue();

        Queue() = delete;
        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;

        // allow only Device to create Queue to ensure that queues are shared
        friend class Device;

        VkQueue _vkQueue;
        uint32_t _queueFamilyIndex;
        uint32_t _queueIndex;
        std::mutex _mutex;
        std::list<FenceAction> _fenceActions;
        uint64_t _submissionNo;
    };
    VSG_type_name(vsg::Queue);

} // namespace vsg
