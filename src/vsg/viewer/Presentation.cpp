/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/Presentation.h>

using namespace vsg;

#include <iostream>

VkResult Presentation::present()
{
    std::cout<<"Presentation::present()"<<std::endl;

    std::vector<VkSemaphore> vk_semaphores;
    for(auto& semaphore : waitSemaphores)
    {
        vk_semaphores.emplace_back(*(semaphore));
    }

    std::vector<VkSwapchainKHR> vk_swapchains;
    std::vector<uint32_t> indices;
    for(auto& window : windows)
    {
        //vk_semaphores.push_back(*(window->frame(window->nextImageIndex()).imageAvailableSemaphore));
        vk_swapchains.emplace_back(*(window->swapchain()));
        indices.emplace_back(window->nextImageIndex());
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vk_semaphores.size());
    presentInfo.pWaitSemaphores = vk_semaphores.data();
    presentInfo.swapchainCount = static_cast<uint32_t>(vk_swapchains.size());
    presentInfo.pSwapchains = vk_swapchains.data();
    presentInfo.pImageIndices = indices.data();

    std::cout<<"pdo.presentInfo->present(..) \n";
    std::cout<<"    presentInfo.waitSemaphoreCount = "<<presentInfo.waitSemaphoreCount<<"\n";
    for(uint32_t i=0; i<presentInfo.waitSemaphoreCount; ++i)
    {
        std::cout<<"        presentInfo.pWaitSemaphores["<<i<<"] = "<<presentInfo.pWaitSemaphores[i]<<"\n";
    }
    std::cout<<"    presentInfo.commandBufferCount = "<<presentInfo.swapchainCount<<"\n";
    for(uint32_t i=0; i<presentInfo.swapchainCount; ++i)
    {
        std::cout<<"        presentInfo.pSwapchains["<<i<<"] = "<<presentInfo.pSwapchains[i]<<"\n";
        std::cout<<"        presentInfo.pImageIndices["<<i<<"] = "<<presentInfo.pImageIndices[i]<<"\n";
    }
    std::cout<<std::endl;

    for(auto& window : windows)
    {
        window->advanceNextImageIndex();
    }

    return queue->present(presentInfo);
}

