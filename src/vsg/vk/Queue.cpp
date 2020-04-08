/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Fence.h>
#include <vsg/vk/Queue.h>

using namespace vsg;

Queue::Queue(VkQueue queue, uint32_t queueFamilyIndex, uint32_t queueIndex) :
    _vkQueue(queue),
    _queueFamilyIndex(queueFamilyIndex),
    _queueIndex(queueIndex)
{
}

Queue::~Queue()
{
}

VkResult Queue::submit(const std::vector<VkSubmitInfo>& submitInfos, Fence* fence)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return vkQueueSubmit(_vkQueue, static_cast<uint32_t>(submitInfos.size()), submitInfos.data(), (fence == nullptr) ? VK_NULL_HANDLE : fence->fence());
}

VkResult Queue::submit(const VkSubmitInfo& submitInfo, Fence* fence)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return vkQueueSubmit(_vkQueue, 1, &submitInfo, (fence == nullptr) ? VK_NULL_HANDLE : fence->fence());
}

VkResult Queue::present(const VkPresentInfoKHR& info)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return vkQueuePresentKHR(_vkQueue, &info);
}

VkResult Queue::waitIdle()
{
    std::lock_guard<std::mutex> guard(_mutex);
    return vkQueueWaitIdle(_vkQueue);
}
