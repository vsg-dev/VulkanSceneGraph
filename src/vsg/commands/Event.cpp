/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/Event.h>
#include <vsg/core/Exception.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Event
//
Event::Event(Device* device, VkEventCreateFlags flags) :
    _device(device)
{
    VkEventCreateInfo eventCreateInfo;
    eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    eventCreateInfo.pNext = nullptr;
    eventCreateInfo.flags = flags;

    if (VkResult result = vkCreateEvent(*device, &eventCreateInfo, nullptr, &_event); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create Event.", result};
    }
}

Event::~Event()
{
    vkDestroyEvent(*_device, _event, nullptr);
}

void Event::set()
{
    vkSetEvent(*_device, _event);
}

void Event::reset()
{
    vkResetEvent(*_device, _event);
}

VkResult Event::status()
{
    return vkGetEventStatus(*_device, _event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// SetEvent
//
SetEvent::SetEvent(ref_ptr<Event> in_event, VkPipelineStageFlags in_stageMask) :
    event(in_event),
    stageMask(in_stageMask)
{
}

SetEvent::~SetEvent()
{
}

void SetEvent::record(CommandBuffer& commandBuffer) const
{
    vkCmdSetEvent(commandBuffer, *event, stageMask);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// ResetEvent
//
ResetEvent::ResetEvent(ref_ptr<Event> in_event, VkPipelineStageFlags in_stageMask) :
    event(in_event),
    stageMask(in_stageMask)
{
}

ResetEvent::~ResetEvent()
{
}

void ResetEvent::record(CommandBuffer& commandBuffer) const
{
    vkCmdResetEvent(commandBuffer, *event, stageMask);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// WaitEvents
//
WaitEvents::WaitEvents() :
    srcStageMask(0),
    dstStageMask(0)
{
}

WaitEvents::~WaitEvents()
{
}

void WaitEvents::record(CommandBuffer& commandBuffer) const
{
    auto& scratchMemory = *(commandBuffer.scratchMemory);

    auto vk_events = scratchMemory.allocate<VkEvent>(events.size());
    for (size_t i = 0; i < events.size(); ++i)
    {
        vk_events[i] = events[i]->vk();
    }

    auto vk_memoryBarriers = scratchMemory.allocate<VkMemoryBarrier>(memoryBarriers.size());
    for (size_t i = 0; i < memoryBarriers.size(); ++i)
    {
        memoryBarriers[i]->assign(commandBuffer, vk_memoryBarriers[i]);
    }

    auto vk_bufferMemoryBarriers = scratchMemory.allocate<VkBufferMemoryBarrier>(bufferMemoryBarriers.size());
    for (size_t i = 0; i < bufferMemoryBarriers.size(); ++i)
    {
        bufferMemoryBarriers[i]->assign(commandBuffer, vk_bufferMemoryBarriers[i]);
    }

    auto vk_imageMemoryBarriers = scratchMemory.allocate<VkImageMemoryBarrier>(imageMemoryBarriers.size());
    for (size_t i = 0; i < imageMemoryBarriers.size(); ++i)
    {
        imageMemoryBarriers[i]->assign(commandBuffer, vk_imageMemoryBarriers[i]);
    }

    vkCmdWaitEvents(
        commandBuffer,
        static_cast<uint32_t>(events.size()),
        vk_events,
        srcStageMask,
        dstStageMask,
        static_cast<uint32_t>(memoryBarriers.size()),
        vk_memoryBarriers,
        static_cast<uint32_t>(bufferMemoryBarriers.size()),
        vk_bufferMemoryBarriers,
        static_cast<uint32_t>(imageMemoryBarriers.size()),
        vk_imageMemoryBarriers);

    scratchMemory.release();
}
