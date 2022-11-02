/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/State.h>

using namespace vsg;

CommandGraph::CommandGraph(ref_ptr<Device> in_device, int family) :
    device(in_device),
    queueFamily(family),
    presentFamily(-1)
{
}

CommandGraph::CommandGraph(ref_ptr<Window> in_window, ref_ptr<Node> child) :
    window(in_window),
    device(in_window->getOrCreateDevice())
{
    VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
    if (window->traits()) queueFlags = window->traits()->queueFlags;

    std::tie(queueFamily, presentFamily) = device->getPhysicalDevice()->getQueueFamily(queueFlags, window->getOrCreateSurface());

    if (child) addChild(child);
}

CommandGraph::~CommandGraph()
{
}

VkCommandBufferLevel CommandGraph::level() const
{
    return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}

void CommandGraph::reset()
{
}

void CommandGraph::record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp, ref_ptr<DatabasePager> databasePager)
{
    if (window && !window->visible())
    {
        return;
    }

    if (!recordTraversal)
    {
        recordTraversal = RecordTraversal::create(nullptr, maxSlot);
    }

    if ((maxSlot + 1) != recordTraversal->getState()->stateStacks.size())
    {
        recordTraversal->getState()->stateStacks.resize(maxSlot + 1);
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
        commandBuffer = cp->allocate(level());
        _commandBuffers.push_back(commandBuffer);
    }
    else
    {
        commandBuffer->reset();
    }

    commandBuffer->numDependentSubmissions().fetch_add(1);

    recordTraversal->getState()->_commandBuffer = commandBuffer;

    // or select index when maps to a dormant CommandBuffer
    VkCommandBuffer vk_commandBuffer = *commandBuffer;

    // need to set up the command
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

    if (camera)
    {
        recordTraversal->setProjectionAndViewMatrix(camera->projectionMatrix->transform(), camera->viewMatrix->transform());
    }

    accept(*recordTraversal);

    vkEndCommandBuffer(vk_commandBuffer);

    recordedCommandBuffers.push_back(commandBuffer);
}

ref_ptr<CommandGraph> vsg::createCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents, bool assignHeadlight)
{
    auto commandGraph = CommandGraph::create(window);

    commandGraph->addChild(createRenderGraphForView(window, camera, scenegraph, contents, assignHeadlight));

    return commandGraph;
}
