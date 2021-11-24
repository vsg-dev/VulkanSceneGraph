#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/PipelineBarrier.h>

namespace vsg
{

    /// Encapsulation of VkEvent
    class VSG_DECLSPEC Event : public Inherit<Object, Event>
    {
    public:
        explicit Event(Device* device, VkEventCreateFlags flags = 0);

        operator VkEvent() const { return _event; }
        VkEvent vk() const { return _event; }

        /// set the state of the vkEvent to signaled
        void set();

        /// set the state of the vkEvent to unsignalled.
        void reset();

        /// get the status of the vkEvent, return VK_EVENT_SET for a signaled event, VK_EVENT_RESET for unsignalled.
        VkResult status();

    protected:
        virtual ~Event();

        VkEvent _event;
        ref_ptr<Device> _device;
    };
    VSG_type_name(vsg::Event);

    using Events = std::vector<ref_ptr<Event>>;

    /// Command class encapsulating vkCmdSetEvent
    class VSG_DECLSPEC SetEvent : public Inherit<Command, SetEvent>
    {
    public:
        SetEvent(ref_ptr<Event> in_event, VkPipelineStageFlags in_stageMask);

        ref_ptr<Event> event;
        VkPipelineStageFlags stageMask;

        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~SetEvent();
    };
    VSG_type_name(vsg::SetEvent);

    /// Command class encapsulating vkCmdReetEvent
    class VSG_DECLSPEC ResetEvent : public Inherit<Command, ResetEvent>
    {
    public:
        ResetEvent(ref_ptr<Event> in_event, VkPipelineStageFlags in_stageMask);

        ref_ptr<Event> event;
        VkPipelineStageFlags stageMask;

        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~ResetEvent();
    };
    VSG_type_name(vsg::ResetEvent);

    /// Command class encapsulating vkCmdWaitEvents
    class VSG_DECLSPEC WaitEvents : public Inherit<Command, WaitEvents>
    {
    public:
        WaitEvents();

        template<typename... Args>
        WaitEvents(VkPipelineStageFlags in_srcStageMask, VkPipelineStageFlags in_destStageMask, Args&&... args) :
            srcStageMask(in_srcStageMask),
            dstStageMask(in_destStageMask)
        {
            (add(args), ...);
        }

        void record(CommandBuffer& commandBuffer) const override;

        void add(ref_ptr<Event> event) { events.emplace_back(event); }
        void add(ref_ptr<MemoryBarrier> mb) { memoryBarriers.emplace_back(mb); }
        void add(ref_ptr<BufferMemoryBarrier> bmb) { bufferMemoryBarriers.emplace_back(bmb); }
        void add(ref_ptr<ImageMemoryBarrier> imb) { imageMemoryBarriers.emplace_back(imb); }

        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;

        Events events;
        MemoryBarriers memoryBarriers;
        BufferMemoryBarriers bufferMemoryBarriers;
        ImageMemoryBarriers imageMemoryBarriers;

    protected:
        virtual ~WaitEvents();
    };
    VSG_type_name(vsg::WaitEvents);

} // namespace vsg
