#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/Presentation.h>
#include <vsg/viewer/RecordAndSubmitTask.h>
#include <vsg/viewer/Window.h>

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/Context.h>

#include <map>

namespace vsg
{

    class VSG_DECLSPEC Viewer : public Inherit<Object, Viewer>
    {
    public:
        Viewer();

        struct PerDeviceObjects
        {
            Windows windows;
            ref_ptr<Queue> graphicsQueue;
            ref_ptr<Queue> presentQueue;
            ref_ptr<Semaphore> renderFinishedSemaphore;

            // cache data to be used each frame
            std::vector<uint32_t> imageIndices;
            std::vector<VkSemaphore> signalSemaphores;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<VkSwapchainKHR> swapchains;
        };

        using DeviceMap = std::map<ref_ptr<Device>, PerDeviceObjects>;

        /// add Window to Viewer
        virtual void addWindow(ref_ptr<Window> window);

        Windows& windows() { return _windows; }
        const Windows& windows() const { return _windows; }

        clock::time_point& start_point() { return _start_point; }
        const clock::time_point& start_point() const { return _start_point; }

        FrameStamp* getFrameStamp() { return _frameStamp; }
        const FrameStamp* getFrameStamp() const { return _frameStamp; }

        /// return true if viewer is valid and active
        bool active() const;

        /// schedule closure of the viewer and associated windows, after a call to Viewer::close() the Viewer::active() method will return false
        void close();

        /// poll the events for all attached windows, return true if new events are available
        bool pollEvents(bool discardPreviousEvents = true);

        /// get the current set of Events that are filled in by prior calls to pollEvents
        Events& getEvents() { return _events; }

        /// get the const current set of Events that are filled in by prior calls to pollEvents
        const Events& getEvents() const { return _events; }

        /// add event handler
        void addEventHandler(ref_ptr<Visitor> eventHandler) { _eventHandlers.emplace_back(eventHandler); }

        void addEventHandlers(EventHandlers&& eventHandlers) { _eventHandlers.splice(_eventHandlers.end(), eventHandlers); }

        /// get the const list of EventHandlers
        EventHandlers& getEventHandlers() { return _eventHandlers; }

        /// get the const list of EventHandlers
        const EventHandlers& getEventHandlers() const { return _eventHandlers; }

        /// convinience method for advancing to the next frame.
        /// Check active status, return false if viewer no longer active.
        /// lf still active poll for pending events and place them in the Events list and advance to the next frame, update generate FrameStamp to signify the advancement to a new frame and return true.
        virtual bool advanceToNextFrame();

        /// poll for pending events and place them in the Events list and update generate FrameStamp to signify the advancement to a new frame.
        virtual void advance();

        /// pass the Events into the any register EventHandlers
        virtual void handleEvents();

        virtual void reassignFrameCache();

        virtual void compile(BufferPreferences bufferPreferences = {});

        virtual bool acquireNextFrame();

        // Manage the work to do each frame uisng RecordAndSubmitTasks. thpse that need to present results to be wired up to respective Presentation object
        using RecordAndSubmitTasks = std::vector<ref_ptr<RecordAndSubmitTask>>;
        RecordAndSubmitTasks recordAndSubmitTasks;

        // Manage the presentation of rendering using Presentation objects
        using Presentations = std::vector<ref_ptr<Presentation>>;
        Presentations presentations;

        void assignRecordAndSubmitTaskAndPresentation(CommandGraphs commandGraphs, DatabasePager* databasePager = nullptr);

        void setupThreading();
        void stopThreading();

        virtual void update();

        virtual void recordAndSubmit();

        virtual void present();

        ref_ptr<CompileTraversal> _compileTraversal;

    protected:
        virtual ~Viewer();

        bool _close = false;
        ref_ptr<ActivityStatus> _status;

        ref_ptr<FrameStamp> _frameStamp;

        Windows _windows;

        DeviceMap _deviceMap;

        clock::time_point _start_point;
        Events _events;
        EventHandlers _eventHandlers;

        bool _threading = false;
        ref_ptr<FrameBlock> _frameBlock;
        ref_ptr<Barrier> _submissionCompleted;
    };

} // namespace vsg
