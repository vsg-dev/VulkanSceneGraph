/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/Viewer.h>

#include <chrono>
#include <iostream>

using namespace vsg;

Viewer::Viewer()
{
    _start_point = clock::now();
}

Viewer::~Viewer()
{
    // don't kill window while devices are still active
    for (auto& pair_pdo : _deviceMap)
    {
        vkDeviceWaitIdle(*pair_pdo.first);
    }
}

void Viewer::addWindow(ref_ptr<Window> window)
{
    _windows.push_back(window);

    ref_ptr<Device> device(window->device());
    PhysicalDevice* physicalDevice = window->physicalDevice();
    if (_deviceMap.find(device) == _deviceMap.end())
    {
        // set up per device settings
        PerDeviceObjects& new_pdo = _deviceMap[device];
        new_pdo.renderFinishedSemaphore = vsg::Semaphore::create(device);
        new_pdo.graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());
        new_pdo.presentQueue = device->getQueue(physicalDevice->getPresentFamily());
        new_pdo.signalSemaphores.push_back(*new_pdo.renderFinishedSemaphore);
    }

    // add per window details to pdo
    PerDeviceObjects& pdo = _deviceMap[device];
    pdo.windows.push_back(window);
    pdo.imageIndices.push_back(0);   // to be filled in by submitFrame()
    pdo.commandBuffers.push_back(0); // to be filled in by submitFrame()
    pdo.waitSemaphores.push_back(*(window->imageAvailableSemaphore()));
    pdo.swapchains.push_back(*(window->swapchain()));
    pdo.waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

bool Viewer::done() const
{
    for (auto window : _windows)
    {
        if (!window->valid())
        {
            // don't exit mainloop while the any devices are still active
            for (auto& pair_pdo : _deviceMap)
            {
                vkDeviceWaitIdle(*pair_pdo.first);
            }
            return true;
        }
    }
    return false;
}

bool Viewer::pollEvents(bool discardPreviousEvents)
{
    bool result = false;

    if (discardPreviousEvents) _events.clear();
    for (auto& window : _windows)
    {
        if (window->pollEvents(_events)) result = true;
    }

    return result;
}

void Viewer::reassignFrameCache()
{
    for (auto& pair_pdo : _deviceMap)
    {
        PerDeviceObjects& pdo = pair_pdo.second;
        pdo.imageIndices.clear();
        pdo.commandBuffers.clear();
        pdo.waitSemaphores.clear();
        pdo.swapchains.clear();
        pdo.waitStages.clear();

        for (auto window : pdo.windows)
        {
            pdo.imageIndices.push_back(0);   // to be filled in by submitFrame()
            pdo.commandBuffers.push_back(0); // to be filled in by submitFrame()
            pdo.waitSemaphores.push_back(*(window->imageAvailableSemaphore()));
            pdo.swapchains.push_back(*(window->swapchain()));
            pdo.waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
    }
}

void Viewer::submitFrame()
{
    bool debugLayersEnabled = false;

    for (auto& window : _windows)
    {
        if (window->debugLayersEnabled()) debugLayersEnabled = true;

        vsg::Semaphore* imageAvailableSemaphore = window->imageAvailableSemaphore();
        VkResult result;
        if (window->resized() || (result = window->acquireNextImage(std::numeric_limits<uint64_t>::max(), *(imageAvailableSemaphore), VK_NULL_HANDLE)) != VK_SUCCESS)
        {
            window->resize();

            reassignFrameCache();

            window->populateCommandBuffers();

            result = window->acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore, VK_NULL_HANDLE);
            if (result != VK_SUCCESS)
            {
                std::cout << "Failed to aquire image VkResult=" << result << std::endl;
                return;
            }
        }
    }

    for (auto& pair_pdo : _deviceMap)
    {
        PerDeviceObjects& pdo = pair_pdo.second;

        // fill in the imageIndices and commandBuffers associated with each window
        for (size_t i = 0; i < pdo.windows.size(); ++i)
        {
            uint32_t imageIndex = pdo.windows[i]->nextImageIndex();
            pdo.imageIndices[i] = imageIndex;
            pdo.commandBuffers[i] = *(pdo.windows[i]->commandBuffer(imageIndex));
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(pdo.waitSemaphores.size());
        submitInfo.pWaitSemaphores = pdo.waitSemaphores.data();
        submitInfo.pWaitDstStageMask = pdo.waitStages.data();

        submitInfo.commandBufferCount = static_cast<uint32_t>(pdo.commandBuffers.size());
        submitInfo.pCommandBuffers = pdo.commandBuffers.data();

        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(pdo.signalSemaphores.size());
        submitInfo.pSignalSemaphores = pdo.signalSemaphores.data();

        if (vkQueueSubmit(pdo.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            std::cout << "Error: failed to submit draw command buffer." << std::endl;
            return;
        }
    }

    for (auto& pair_pdo : _deviceMap)
    {
        PerDeviceObjects& pdo = pair_pdo.second;

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(pdo.signalSemaphores.size());
        presentInfo.pWaitSemaphores = pdo.signalSemaphores.data();
        presentInfo.swapchainCount = static_cast<uint32_t>(pdo.swapchains.size());
        presentInfo.pSwapchains = pdo.swapchains.data();
        presentInfo.pImageIndices = pdo.imageIndices.data();

        vkQueuePresentKHR(pdo.presentQueue, &presentInfo);
    }

    if (debugLayersEnabled)
    {
        for (auto& pair_pdo : _deviceMap)
        {
            PerDeviceObjects& pdo = pair_pdo.second;
            vkQueueWaitIdle(pdo.presentQueue);
        }
    }
}
