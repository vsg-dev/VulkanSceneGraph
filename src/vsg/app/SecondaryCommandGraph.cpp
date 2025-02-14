/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/commands/ExecuteCommands.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/lighting/Light.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/State.h>

using namespace vsg;

SecondaryCommandGraph::SecondaryCommandGraph(ref_ptr<Device> in_device, int family) :
    Inherit(in_device, family)
{
}

SecondaryCommandGraph::SecondaryCommandGraph(ref_ptr<Window> in_window, ref_ptr<Node> child, uint32_t in_subpass) :
    Inherit(in_window, child),
    subpass(in_subpass)
{
}

SecondaryCommandGraph::~SecondaryCommandGraph()
{
}

VkCommandBufferLevel SecondaryCommandGraph::level() const
{
    return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
}

void SecondaryCommandGraph::reset()
{
    for (auto& ec : _executeCommands) ec->reset();
}

void SecondaryCommandGraph::_connect(ExecuteCommands* ec)
{
    _executeCommands.emplace_back(ec);
}

void SecondaryCommandGraph::_disconnect(ExecuteCommands* ec)
{
    auto itr = std::find(_executeCommands.begin(), _executeCommands.end(), ec);
    if (itr != _executeCommands.end()) _executeCommands.erase(itr);
}

RenderPass* SecondaryCommandGraph::getRenderPass()
{
    if (renderPass)
    {
        return renderPass;
    }
    else if (framebuffer)
    {
        return framebuffer->getRenderPass();
    }
    else if (window)
    {
        return window->getOrCreateRenderPass();
    }
    return nullptr;
}

void SecondaryCommandGraph::record(ref_ptr<RecordedCommandBuffers> recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp, ref_ptr<DatabasePager> databasePager)
{
    if (window && !window->visible())
    {
        return;
    }

    if (!recordTraversal)
        recordTraversal = RecordTraversal::create(maxSlot);
    else
        recordTraversal->getState()->reserve(maxSlot);

    recordTraversal->recordedCommandBuffers = recordedCommandBuffers;
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

    // need to set up the command buffer
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    if (_executeCommands.size() > 1) beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkCommandBufferInheritanceInfo inheritanceInfo;
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext = nullptr;
    inheritanceInfo.occlusionQueryEnable = occlusionQueryEnable;
    inheritanceInfo.queryFlags = queryFlags;
    inheritanceInfo.pipelineStatistics = pipelineStatistics;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    if (const auto activeRenderPass = getRenderPass())
        inheritanceInfo.renderPass = *(activeRenderPass);
    else
        inheritanceInfo.renderPass = VK_NULL_HANDLE;

    inheritanceInfo.subpass = subpass;

    if (framebuffer)
    {
        inheritanceInfo.framebuffer = *framebuffer;
    }
    else if (window)
    {
        //inheritanceInfo.framebuffer = *(window->framebuffer(window->nextImageIndex()));
        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
    }

    vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

    traverse(*recordTraversal);

    vkEndCommandBuffer(vk_commandBuffer);

    // pass on this command buffer to connected ExecuteCommands nodes
    for (auto& ec : _executeCommands)
    {
        ec->completed(*this, commandBuffer);
    }

    recordedCommandBuffers->add(submitOrder, commandBuffer);
}

ref_ptr<SecondaryCommandGraph> vsg::createSecondaryCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, uint32_t subpass, bool assignHeadlight)
{
    // set up the view
    auto view = View::create(camera);
    if (assignHeadlight) view->addChild(createHeadlight());
    if (scenegraph) view->addChild(scenegraph);

    auto commandGraph = SecondaryCommandGraph::create(window, view, subpass);

    return commandGraph;
}
