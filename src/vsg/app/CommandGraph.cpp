/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/State.h>

using namespace vsg;

CommandGraph::CommandGraph()
{
    CPU_INSTRUMENTATION_L1(instrumentation);
}

CommandGraph::CommandGraph(ref_ptr<Device> in_device, int family) :
    device(in_device),
    queueFamily(family),
    presentFamily(-1)
{
    CPU_INSTRUMENTATION_L1(instrumentation);
}

CommandGraph::CommandGraph(ref_ptr<Window> in_window, ref_ptr<Node> child) :
    window(in_window),
    device(in_window->getOrCreateDevice())
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
    if (window->traits()) queueFlags = window->traits()->queueFlags;

    std::tie(queueFamily, presentFamily) = device->getPhysicalDevice()->getQueueFamily(queueFlags, window->getOrCreateSurface());

    if (child) addChild(child);
}

CommandGraph::~CommandGraph()
{
    CPU_INSTRUMENTATION_L1(instrumentation);
}

VkCommandBufferLevel CommandGraph::level() const
{
    return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}

void CommandGraph::reset()
{
}

ref_ptr<RecordTraversal> CommandGraph::getOrCreateRecordTraversal()
{
    //CPU_INSTRUMENTATION_L1(instrumentation);
    if (!recordTraversal)
        recordTraversal = RecordTraversal::create(maxSlot);
    else
        recordTraversal->getState()->reserve(maxSlot);

    return recordTraversal;
}

void CommandGraph::record(ref_ptr<RecordedCommandBuffers> recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp, ref_ptr<DatabasePager> databasePager)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "CommandGraph record", COLOR_RECORD_L1);

    if (window && !window->visible())
    {
        //vsg::info("CommandGraph::record() ", frameStamp->frameCount, " window not visible.");
        return;
    }

    // create the RecordTraversal if it isn't already created
    getOrCreateRecordTraversal();

    recordTraversal->recordedCommandBuffers = recordedCommandBuffers;
    recordTraversal->setFrameStamp(frameStamp);
    recordTraversal->setDatabasePager(databasePager);
    recordTraversal->clearBins();
    recordTraversal->regionsOfInterest.clear();

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

    // need to set up the command buffer
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

    {
        COMMAND_BUFFER_INSTRUMENTATION(instrumentation, *commandBuffer, "CommandGraph record", COLOR_RECORD)
        traverse(*recordTraversal);
    }

    vkEndCommandBuffer(vk_commandBuffer);

    recordedCommandBuffers->add(submitOrder, commandBuffer);
}

ref_ptr<CommandGraph> vsg::createCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents, bool assignHeadlight)
{
    auto commandGraph = CommandGraph::create(window);

    commandGraph->addChild(createRenderGraphForView(window, camera, scenegraph, contents, assignHeadlight));

    return commandGraph;
}
