#include <vsg/viewer/Window.h>

#include <vsg/vk/CommandVisitor.h>

#include <array>
#include <iostream>
#include <chrono>

namespace vsg
{

Window::Window() :
    _clearColor{0.2f, 0.2f, 0.4f, 1.0f},
    _debugLayersEnabled(false)
{
}

Window::~Window()
{
    // do we need to call clear()?
}

void Window::clear()
{
    std::cout<<"vsg::Window::clear() start"<<std::endl;

    _frames.clear();
    _swapchain = 0;

    _depthImage = 0;
    _depthImageMemory = 0;
    _depthImageView = 0;

    _renderPass = 0;
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
    if (!_imageAvailableSemaphore)
    {
        _imageAvailableSemaphore = vsg::Semaphore::create(_device);
    }

    if (_swapchain)
    {
        // make sure all operations on the device have stopped before we go deleting associated resources
        vkDeviceWaitIdle(*_device);

        // clean up previous swap chain before we begin creating a new one.
        _frames.clear();

        _depthImageView = 0;
        _depthImage = 0;
        _depthImageMemory = 0;

        _swapchain = 0;
    }

    // is width and height even required here as the surface appear to control it.
    _swapchain = Swapchain::create(_physicalDevice, _device, _surface, width, height);

    // pass back the extents used by the swap chain.
    _extent2D = _swapchain->getExtent();

    // create depth buffer
    //VkFormat depthFormat = VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    VkImageCreateInfo depthImageCreateInfo = {};
    depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageCreateInfo.extent.width = _extent2D.width;
    depthImageCreateInfo.extent.height = _extent2D.height;
    depthImageCreateInfo.extent.depth = 1;
    depthImageCreateInfo.mipLevels = 1;
    depthImageCreateInfo.arrayLayers = 1;
    depthImageCreateInfo.format = depthFormat;
    depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    _depthImage = Image::create(_device, depthImageCreateInfo);
    _depthImageMemory = DeviceMemory::create(_physicalDevice, _device, _depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkBindImageMemory(*_device, *_depthImage, *_depthImageMemory, 0);

    _depthImageView = ImageView::create(_device, _depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT);

    // set up framebuffer and associated resources
    Swapchain::ImageViews& imageViews = _swapchain->getImageViews();

    for (size_t i=0; i<imageViews.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = { *imageViews[i], *_depthImageView };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _extent2D.width;
        framebufferInfo.height = _extent2D.height;
        framebufferInfo.layers = 1;

        ref_ptr<Framebuffer> fb = Framebuffer::create(_device, framebufferInfo);
        ref_ptr<CommandPool> cp = CommandPool::create(_device, _physicalDevice->getGraphicsFamily());
        ref_ptr<CommandBuffer> cb =  CommandBuffer::create(_device, cp, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        _frames.push_back( {imageViews[i], fb, cp, cb} );
    }

    dispatchCommandsToQueue(_device, _frames[0].commandPool, _device->getQueue(_physicalDevice->getGraphicsFamily()), [&](VkCommandBuffer commandBuffer)
    {

        vsg::ImageMemoryBarrier depthImageMemoryBarrier(
                        0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        _depthImage);

        depthImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT;

        depthImageMemoryBarrier.cmdPiplineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

    });
}

void Window::populateCommandBuffers(vsg::Node* commandGraph)
{
    for(auto& frame : _frames)
    {
        CommandVisitor cv(frame.framebuffer, *frame.commandBuffer, _extent2D, _clearColor);
        cv.populateCommandBuffer(commandGraph);
    }
}

}