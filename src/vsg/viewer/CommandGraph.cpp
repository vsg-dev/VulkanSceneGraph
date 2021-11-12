/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/traversals/RecordTraversal.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/ExecuteCommands.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

CommandGraph::CommandGraph(Device* in_device, int family) :
    device(in_device),
    queueFamily(family),
    presentFamily(-1)
{
}

CommandGraph::CommandGraph(Window* in_window) :
    window(in_window),
    device(in_window->getOrCreateDevice())
{
    VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
    if (window->traits()) queueFlags = window->traits()->queueFlags;

    std::tie(queueFamily, presentFamily) = device->getPhysicalDevice()->getQueueFamily(queueFlags, window->getOrCreateSurface());

    for (size_t i = 0; i < window->numFrames(); ++i)
    {
        ref_ptr<CommandPool> cp = CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        _commandBuffers.emplace_back(CommandBuffer::create(device, cp, level));
    }
}

CommandGraph::~CommandGraph()
{
}

void CommandGraph::reset()
{
    for (auto& ec : _executeCommands) ec->reset();
}

void CommandGraph::_connect(ExecuteCommands* ec)
{
    _executeCommands.emplace_back(ec);
}

void CommandGraph::_disconnect(ExecuteCommands* ec)
{
    auto itr = std::find(_executeCommands.begin(), _executeCommands.end(), ec);
    if (itr != _executeCommands.end()) _executeCommands.erase(itr);
}

void CommandGraph::record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp, ref_ptr<DatabasePager> databasePager)
{
    if (window && !window->visible())
    {
        return;
    }

    if (!recordTraversal)
    {
        recordTraversal = new RecordTraversal(nullptr, maxSlot);
    }

    recordTraversal->setFrameStamp(frameStamp);
    recordTraversal->setDatabasePager(databasePager);
    recordTraversal->clearBins();

    ref_ptr<CommandBuffer> commandBuffer;
    for (auto& cb : _commandBuffers)
    {
        if (cb->numDependentSubmissions() == 0)
        {
            commandBuffer = cb;
            break;
        }
    }
    if (!commandBuffer)
    {
        ref_ptr<CommandPool> cp = CommandPool::create(device, queueFamily);
        commandBuffer = CommandBuffer::create(device, cp, level);
        _commandBuffers.push_back(commandBuffer);
    }
    else
    {
        commandBuffer->getCommandPool()->reset();
    }

    commandBuffer->numDependentSubmissions().fetch_add(1);

    recordTraversal->getState()->_commandBuffer = commandBuffer;

    // or select index when maps to a dormant CommandBuffer
    VkCommandBuffer vk_commandBuffer = *commandBuffer;

    // need to set up the command
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo
    VkCommandBufferBeginInfo beginInfo = {};
    VkCommandBufferInheritanceInfo inheritanceInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
    {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.pNext = nullptr;
        inheritanceInfo.occlusionQueryEnable = occlusionQueryEnable;
        inheritanceInfo.queryFlags = queryFlags;
        inheritanceInfo.pipelineStatistics = pipelineStatistics;

        if (window)
        {
            inheritanceInfo.renderPass = *(window->getRenderPass());
            inheritanceInfo.subpass = subpass;
            //inheritanceInfo.framebuffer = *(window->framebuffer(window->nextImageIndex()));
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;
        }

        beginInfo.pInheritanceInfo = &inheritanceInfo;
    }
    else
    {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;
    }

    vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

    if (camera)
    {
        recordTraversal->setProjectionAndViewMatrix(camera->projectionMatrix->transform(), camera->viewMatrix->transform());
    }

    accept(*recordTraversal);

    vkEndCommandBuffer(vk_commandBuffer);

    if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
    {
        // pass on this command buffer to connected ExecuteCommands nodes
        for (auto& ec : _executeCommands)
        {
            ec->completed(commandBuffer);
        }
    }

    recordedCommandBuffers.push_back(commandBuffer);
}

ref_ptr<CommandGraph> vsg::createCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents)
{
    auto commandGraph = CommandGraph::create(window);

    commandGraph->addChild(createRenderGraphForView(window, camera, scenegraph, contents));

    return commandGraph;
}

ref_ptr<CommandGraph> vsg::createSecondaryCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, uint32_t subpass)
{
    auto commandGraph = CommandGraph::create(window);

    commandGraph->camera = camera;
    commandGraph->level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    commandGraph->subpass = subpass;

    commandGraph->addChild(ref_ptr<Node>(scenegraph));

    return commandGraph;
}
