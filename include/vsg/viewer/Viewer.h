#pragma once

#include <vsg/viewer/Window.h>

#include <map>

namespace vsg
{

    class Viewer : public Object
    {
    public:
        Viewer();

        using Windows = std::vector<ref_ptr<Window>>;

        struct PerDeviceObjects
        {
            Windows                             windows;
            VkQueue                             graphicsQueue;
            VkQueue                             presentQueue;
            ref_ptr<Semaphore>                  renderFinishedSemaphore;

            // cache data to used each frame
            std::vector<uint32_t>               imageIndices;
            std::vector<VkSemaphore>            waitSemaphores;
            std::vector<VkSemaphore>            signalSemaphores;
            std::vector<VkCommandBuffer>        commandBuffers;
            std::vector<VkSwapchainKHR>         swapchains;
            std::vector<VkPipelineStageFlags>   waitStages;
        };

        using DeviceMap = std::map<ref_ptr<Device>, PerDeviceObjects>;

        void addWindow(Window* window);

        Windows& windows() { return _windows; }
        const Windows& windows() const { return _windows; }

        bool done() const;

        bool pollEvents();

        void submitFrame(vsg::Node* commandGraph);

        void reassignFrameCache();

    protected:
        virtual ~Viewer();

        Windows             _windows;
        DeviceMap           _deviceMap;
        ref_ptr<Semaphore>  _renderFinishedSemaphore;
    };

}
