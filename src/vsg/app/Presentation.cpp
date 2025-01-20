/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Presentation.h>
#include <vsg/io/Logger.h>

using namespace vsg;

VkResult Presentation::present()
{
    //debug("Presentation::present()");

    std::vector<VkSemaphore> vk_semaphores;
    for (auto& semaphore : waitSemaphores)
    {
        vk_semaphores.emplace_back(*(semaphore));
    }

    std::vector<VkSwapchainKHR> vk_swapchains;
    std::vector<uint32_t> indices;
    for (auto& window : windows)
    {
        size_t imageIndex = window->imageIndex();
        if (window->visible() && imageIndex < window->numFrames())
        {
            vk_swapchains.emplace_back(*(window->getOrCreateSwapchain()));
            indices.emplace_back(static_cast<uint32_t>(imageIndex));
        }
    }

    if (vk_swapchains.empty())
    {
        // nothing to present so return early
        return VK_SUCCESS;
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vk_semaphores.size());
    presentInfo.pWaitSemaphores = vk_semaphores.data();
    presentInfo.swapchainCount = static_cast<uint32_t>(vk_swapchains.size());
    presentInfo.pSwapchains = vk_swapchains.data();
    presentInfo.pImageIndices = indices.data();

#if 0
    debug( "pdo.presentInfo->present(..)");
    debug( "    presentInfo.waitSemaphoreCount = ", presentInfo.waitSemaphoreCount);
    for (uint32_t i = 0; i < presentInfo.waitSemaphoreCount; ++i)
    {
        debug( "        presentInfo.pWaitSemaphores[", i, "] = ", presentInfo.pWaitSemaphores[i]);
    }
    debug( "    presentInfo.commandBufferCount = ", presentInfo.swapchainCount);
    for (uint32_t i = 0; i < presentInfo.swapchainCount; ++i)
    {
        debug( "        presentInfo.pSwapchains[", i, "] = ", presentInfo.pSwapchains[i]);
        debug( "        presentInfo.pImageIndices[", i, "] = ", presentInfo.pImageIndices[i]);
    }
    debug("\n");
#endif

    return queue->present(presentInfo);
}
