#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/Window.h>
#include <vsg/viewer/View.h>

#include <map>

namespace vsg
{

    class VSG_DECLSPEC Viewer : public Inherit<Object, Viewer>
    {
    public:
        Viewer();

        using Windows = std::vector<ref_ptr<Window>>;
        using Views = std::vector<ref_ptr<View>>;

        struct PerDeviceObjects
        {
            Windows windows;
            VkQueue graphicsQueue;
            VkQueue presentQueue;
            ref_ptr<Semaphore> renderFinishedSemaphore;

            // cache data to used each frame
            std::vector<uint32_t> imageIndices;
            std::vector<VkSemaphore> waitSemaphores;
            std::vector<VkSemaphore> signalSemaphores;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<VkSwapchainKHR> swapchains;
            std::vector<VkPipelineStageFlags> waitStages;
        };

        using DeviceMap = std::map<ref_ptr<Device>, PerDeviceObjects>;


        /// add Window to Viewer
        void addWindow(ref_ptr<Window> window);

        Windows& windows() { return _windows; }
        const Windows& windows() const { return _windows; }



        /// add View to Viewer
        void addView(ref_ptr<View> view);

        Views& getViews() { return _views; }
        const Views& getViews() const { return _views; }


        clock::time_point& start_point() { return _start_point; }
        const clock::time_point& start_point() const { return _start_point; }

        bool done() const;

        bool pollEvents();

        void submitFrame();

        void reassignFrameCache();

    protected:
        virtual ~Viewer();

        Windows _windows;
        Views   _views;

        DeviceMap _deviceMap;
        ref_ptr<Semaphore> _renderFinishedSemaphore;
        clock::time_point _start_point;
    };

} // namespace vsg
