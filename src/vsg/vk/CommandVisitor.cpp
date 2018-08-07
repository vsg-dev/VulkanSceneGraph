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



CommandVisitor::CommandVisitor(Framebuffer* framebuffer, RenderPass* renderPass, VkCommandBuffer commandBuffer, const VkExtent2D& extent, const VkClearColorValue& clearColor) :
    _framebuffer(framebuffer),
    _renderPass(renderPass),
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

void CommandVisitor::populateCommandBuffer(vsg::Node* subgraph)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = *_renderPass;
        renderPassInfo.framebuffer = *_framebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _extent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = _clearColor;
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            subgraph->accept(*this);

        vkCmdEndRenderPass(_commandBuffer);

    vkEndCommandBuffer(_commandBuffer);
}

}