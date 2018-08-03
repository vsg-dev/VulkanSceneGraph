#include <vsg/vk/CommandVisitor.h>

#include <array>

#include <iostream>
#include <typeinfo>

namespace vsg
{


template<typename T>
struct StoreAndRestore
{
    using P = T*;

    T _value;
    P _ptr;

    StoreAndRestore(T& value) : _value(value), _ptr(&value) {}
    ~StoreAndRestore() { *_ptr = _value; }
};



CommandVisitor::CommandVisitor(Framebuffer* framebuffer, VkCommandBuffer commandBuffer, const VkExtent2D& extent, const VkClearColorValue& clearColor) :
    _framebuffer(framebuffer),
    _commandBuffer(commandBuffer),
    _extent(extent),
    _clearColor(clearColor)
{
}

void CommandVisitor::apply(Object& object)
{
    std::cout<<"Visiting internal object : "<<typeid(object).name()<<std::endl;
    object.traverse(*this);
}

void CommandVisitor::apply(Node& object)
{
    std::cout<<"Visiting internal node : "<<typeid(object).name()<<std::endl;
    object.traverse(*this);
}

void CommandVisitor::apply(Command& cmd)
{
    std::cout<<"Visiting leaf node : "<<typeid(cmd).name()<<std::endl;
    cmd.dispatch(_commandBuffer);
}

void CommandVisitor::apply(RenderPass& renderPass)
{
    std::cout<<"Visiting RenderPass : "<<typeid(renderPass).name()<<std::endl;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = *_framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _extent;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = _clearColor;
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        renderPass.traverse(*this);

    vkCmdEndRenderPass(_commandBuffer);
}

void CommandVisitor::apply(CommandBuffer& commandBuffer)
{
    std::cout<<"Visiting CommandBuffer : "<<typeid(commandBuffer).name()<<std::endl;

    StoreAndRestore<VkCommandBuffer> temp(_commandBuffer);

    // make this CommandBuffer the current one to use for all operations
    _commandBuffer = commandBuffer;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = commandBuffer.flags();
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

        commandBuffer.traverse(*this);

    vkEndCommandBuffer(commandBuffer);

    std::cout<<"End visit CommandBuffer"<<std::endl;
}

void CommandVisitor::populateCommandBuffer(vsg::Node* subgraph)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);

        subgraph->accept(*this);

    vkEndCommandBuffer(_commandBuffer);
}

}