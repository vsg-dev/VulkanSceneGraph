#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <any>

#include <vsg/vk/RenderPass.h>
#include <vsg/vk/Swapchain.h>

namespace vsg
{
    // forward declare
    class Window;

    class WindowTraits : public Inherit<Object, WindowTraits>
    {
    public:
        WindowTraits() {}
        WindowTraits(const WindowTraits&) = delete;
        WindowTraits& operator=(const WindowTraits&) = delete;

        WindowTraits(int32_t in_x, int32_t in_y, uint32_t in_width, uint32_t in_height) :
            x(in_x),
            y(in_y),
            width(in_width),
            height(in_height) {}

        WindowTraits(uint32_t in_width, uint32_t in_height) :
            width(in_width),
            height(in_height) {}

        int32_t x = 0;
        int32_t y = 0;
        uint32_t width = 1280;
        uint32_t height = 1024;

        bool fullscreen = false;

        std::string display;
        uint32_t screenNum = 0;

        std::string windowClass = "vsg::Window";
        std::string windowTitle = "vsg window";

        bool decoration = true;
        bool hdpi = true;

        // X11 hint of whether to ignore the Window managers redirection of window size/position
        bool overrideRedirect = false;

        bool debugLayer = false;
        bool apiDumpLayer = false;

        SwapchainPreferences swapchainPreferences;

        VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
        VkPipelineStageFlagBits imageAvailableSemaphoreWaitFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        vsg::Names instanceExtensionNames;
        vsg::Names deviceExtensionNames;

        ref_ptr<Device> device;
        ref_ptr<RenderPass> renderPass;

        Window* shareWindow = nullptr;

        AllocationCallbacks* allocator = nullptr;

        std::any nativeHandle;
        void* nativeWindow = nullptr;

    protected:
        virtual ~WindowTraits() {}
    };

} // namespace vsg
