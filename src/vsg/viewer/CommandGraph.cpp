/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield
Copyright(c) 2020 Julien Valentin

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/traversals/RecordTraversal.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/vk/State.h>
using namespace vsg;

CommandGraph::CommandGraph(Device* device, int family) :
    _device(device),
    _queueFamily(family),
    _presentFamily(-1)
{
}

CommandGraph::CommandGraph(Window* window)
{
    if (window)
    {
        windows.emplace_back(window);

        _device = window->device();

        VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
        if (window->traits()) queueFlags = window->traits()->queueFlags;

        std::tie(_queueFamily, _presentFamily) = window->physicalDevice()->getQueueFamily(queueFlags, window->surface());

        for (size_t i = 0; i < window->numFrames(); ++i)
        {
            commandBuffers.emplace_back(window->commandBuffer(i));
        }
    }
}

void CommandGraph::record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp, ref_ptr<DatabasePager> databasePager)
{

    if (!recordTraversal)
    {
        recordTraversal = new RecordTraversal(nullptr, _maxSlot);
    }

    //useless?
    if(recordTraversal->frameStamp == frameStamp)
        return;

    /// wait primary consumption if secondary
    for(auto& primMutex : _primaryMutices)
    {
        primMutex->lock();
    }

    recordTraversal->frameStamp = frameStamp;
    recordTraversal->databasePager = databasePager;
    if (databasePager) recordTraversal->culledPagedLODs = databasePager->culledPagedLODs;

    ref_ptr<CommandBuffer> commandBuffer;
    for (auto& cb : commandBuffers)
    {
        if (cb->numDependentSubmissions() == 0)
        {
            commandBuffer = cb;
        }
    }
    if (!commandBuffer)
    {
        ref_ptr<CommandPool> cp = CommandPool::create(_device, _queueFamily);
        commandBuffer = CommandBuffer::create(_device, cp, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, _commandBuffersLevel);
        commandBuffers.push_back(commandBuffer);
    }

    commandBuffer->numDependentSubmissions().fetch_add(1);
    lastRecorded = commandBuffer;
    recordTraversal->state->_commandBuffer = commandBuffer;

    // or select index when maps to a dormant CommandBuffer
    VkCommandBuffer vk_commandBuffer = *commandBuffer;

    // need to set up the command
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = _commandBuffersLevel==VK_COMMAND_BUFFER_LEVEL_PRIMARY?
                VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT :
               VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    VkCommandBufferInheritanceInfo inherit;
    inherit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inherit.renderPass = * windows[0]->renderPass();
    inherit.subpass = _subpassIndex;
    inherit.framebuffer = VK_NULL_HANDLE;
    inherit.occlusionQueryEnable = VK_FALSE;
    inherit.queryFlags = 0; //VK_QUERY_CONTROL_PRECISE_BIT;
    inherit.pipelineStatistics = 0;
    inherit.pNext = nullptr;
    if(_commandBuffersLevel != VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        beginInfo.pInheritanceInfo = &inherit;

    vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

    accept(*recordTraversal);

    vkEndCommandBuffer(vk_commandBuffer);

    recordedCommandBuffers.push_back(recordTraversal->state->_commandBuffer);

    for(auto& secondaryMutex : _secondaryMutices)
        secondaryMutex->unlock();
}

ref_ptr<CommandGraph> vsg::createCommandGraphForView(Window* window, Camera* camera, Node* scenegraph, VkCommandBufferLevel lev, uint sub, VkSubpassContents content)
{
    ref_ptr<CommandGraph> commandGraph;
    auto [graphicsFamily, presentFamily] = window->physicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT, window->surface());

    if(lev == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        commandGraph = CommandGraph::create(window);
        auto renderGraph = vsg::RenderGraph::create();

        renderGraph->addChild(ref_ptr<Node>(scenegraph));
        renderGraph->content = content;
        renderGraph->camera = camera;
        renderGraph->window = window;

        renderGraph->renderArea.offset = {0, 0};
        renderGraph->renderArea.extent = window->extent2D();

        renderGraph->clearValues.resize(2);
        renderGraph->clearValues[0].color = window->clearColor();
        renderGraph->clearValues[1].depthStencil = VkClearDepthStencilValue{1.0f, 0};

        commandGraph->addChild(renderGraph);
    }
    else
    {
        commandGraph = CommandGraph::create(window->device(), graphicsFamily);
        commandGraph->windows.emplace_back(window);
        commandGraph->addChild(ref_ptr<Node>(scenegraph));
    }

    commandGraph->_commandBuffersLevel = lev;
    commandGraph->_subpassIndex = sub;

     return commandGraph;
}
