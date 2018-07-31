#include <vsg/vk/CommandVisitor.h>
#include <vsg/vk/Window.h>

#include <iostream>

namespace vsg
{

Window::Window() {}

Window::~Window()
{
    // do we need to call clear()?
}

void Window::populateCommandBuffers(vsg::Node* commandGraph)
{
    for(auto& frame : _frames)
    {
        CommandVisitor cv(frame.framebuffer, *frame.commandBuffer, _swapchain->getExtent(), VkClearValue{0.2f, 0.2f, 0.4f, 1.0f});
        cv.populateCommandBuffer(commandGraph);
    }
}

void Window::clear()
{
    std::cout<<"vsg::Window::clear() start"<<std::endl;

    _frames.clear();
    _renderPass = 0;
    _swapchain = 0;
    _surface = 0;
    _device = 0;
    _physicalDevice = 0;

    std::cout<<"vsg::Window::clear() end"<<std::endl;
}

void Window::share(const Window& window)
{
    _instance = window._instance;
    _physicalDevice = window._physicalDevice;
    _device = window._device;
    _renderPass = window._renderPass;
}

void Window::buildSwapchain(uint32_t width, uint32_t height)
{
    _extent2D.width = width;
    _extent2D.height = height;

    std::cout<<"buildSwapchain("<<width<<", "<<height<<")"<<std::endl;

    if (_swapchain)
    {
        std::cout<<"cleaning up old swapchain objects"<<std::endl;
        _frames.clear();
        _swapchain = 0;
        std::cout<<"creating new swapchain objects"<<std::endl;
    }

    _swapchain = Swapchain::create(_physicalDevice, _device, _surface, width, height);

    Swapchain::ImageViews& imageViews = _swapchain->getImageViews();
    Framebuffers frameBuffers = createFrameBuffers(_device, _swapchain, _renderPass);

    std::size_t size = std::min(imageViews.size(), frameBuffers.size());
    for (size_t i=0; i<size; ++i)
    {
        ref_ptr<CommandPool> cp = CommandPool::create(_device, _physicalDevice->getGraphicsFamily());
        ref_ptr<CommandBuffer> cb =  CommandBuffer::create(_device, cp, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        _frames.push_back( {imageViews[i], frameBuffers[i], cp, cb} );
    }

    std::cout<<"finished buildSwapchain()"<<std::endl;

}

}