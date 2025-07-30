/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Window.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/core/Exception.h>
#include <vsg/core/Version.h>
#include <vsg/io/Logger.h>
#include <vsg/maths/color.h>
#include <vsg/maths/vec4.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/CoordinateSpace.h>
#include <vsg/vk/SubmitCommands.h>

#include <array>
#include <chrono>

using namespace vsg;

#if VSG_SUPPORTS_Windowing == 0
ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits>)
{
    return {};
}
#endif

Window::Window(ref_ptr<WindowTraits> traits) :
    _traits(traits),
    _extent2D{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()},
    _clearColor{0.2f, 0.2f, 0.4f, 1.0f},
    _framebufferSamples(VK_SAMPLE_COUNT_1_BIT)
{
    if (_traits && (_traits->swapchainPreferences.surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB || _traits->swapchainPreferences.surfaceFormat.format == VK_FORMAT_B8G8R8_SRGB))
    {
        _clearColor = sRGB_to_linear(_clearColor);
    }
}

Window::~Window()
{
}

void Window::clear()
{
    _frames.clear();
    _swapchain.reset();

    _depthImage.reset();
    _depthImageView.reset();

    _renderPass.reset();
    _surface.reset();
    _device.reset();
    _physicalDevice.reset();
}

void Window::share(ref_ptr<Device> device)
{
    setDevice(device);
    _initSurface();
    _initFormats();
    _initRenderPass();
}

VkSurfaceFormatKHR Window::surfaceFormat()
{
    if (!_device) _initDevice();
    return _imageFormat;
}

VkFormat Window::depthFormat()
{
    if (!_device) _initDevice();
    return _depthFormat;
}

void Window::setInstance(ref_ptr<Instance> instance)
{
    _instance = instance;
}

ref_ptr<Instance> Window::getOrCreateInstance()
{
    if (!_instance) _initInstance();
    return _instance;
}

void Window::setSurface(ref_ptr<Surface> surface)
{
    _surface = surface;
}

ref_ptr<Surface> Window::getOrCreateSurface()
{
    if (!_surface) _initSurface();
    return _surface;
}

void Window::setPhysicalDevice(ref_ptr<PhysicalDevice> physicalDevice)
{
    _physicalDevice = physicalDevice;
}

ref_ptr<PhysicalDevice> Window::getOrCreatePhysicalDevice()
{
    if (!_physicalDevice) _initPhysicalDevice();
    return _physicalDevice;
}

void Window::setDevice(ref_ptr<Device> device)
{
    _device = device;
    if (_device)
    {
        _physicalDevice = _device->getPhysicalDevice();
        _instance = _device->getInstance();
    }
}

ref_ptr<Device> Window::getOrCreateDevice()
{
    if (!_device) _initDevice();
    return _device;
}

void Window::setRenderPass(ref_ptr<RenderPass> renderPass)
{
    _renderPass = renderPass;
}

ref_ptr<RenderPass> Window::getOrCreateRenderPass()
{
    if (!_renderPass) _initRenderPass();
    return _renderPass;
}

ref_ptr<Swapchain> Window::getOrCreateSwapchain()
{
    if (!_swapchain) _initSwapchain();
    return _swapchain;
}

ref_ptr<Image> Window::getOrCreateDepthImage()
{
    if (!_depthImage) _initSwapchain();
    return _depthImage;
}

ref_ptr<ImageView> Window::getOrCreateDepthImageView()
{
    if (!_depthImageView) _initSwapchain();
    return _depthImageView;
}

void Window::_initInstance()
{
    // create the vkInstance
    _traits->validate();

    vsg::Names& instanceExtensions = _traits->instanceExtensionNames;

    instanceExtensions.push_back("VK_KHR_surface");
    instanceExtensions.push_back(instanceExtensionSurfaceName());

    _instance = vsg::Instance::create(instanceExtensions, _traits->requestedLayers, _traits->vulkanVersion);
}

void Window::_initPhysicalDevice()
{
    if (!_instance) _initInstance();
    if (!_surface) _initSurface();

    // if required set up physical device
    if (!_physicalDevice)
    {
        _physicalDevice = _instance->getPhysicalDevice(_traits->queueFlags, _surface, _traits->deviceTypePreferences);
        if (!_physicalDevice) throw Exception{"Error: vsg::Window::create(...) failed to create Window,  no suitable Vulkan PhysicalDevice available.", VK_ERROR_INVALID_EXTERNAL_HANDLE};
    }
}

void Window::_initFormats()
{
    vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*_physicalDevice, *_surface);

    _imageFormat = vsg::selectSwapSurfaceFormat(supportDetails, _traits->swapchainPreferences.surfaceFormat);
    _depthFormat = _traits->depthFormat;

    // compute the sample bits to use
    if (_traits->samples != VK_SAMPLE_COUNT_1_BIT)
    {
        VkSampleCountFlags deviceColorSamples = _physicalDevice->getProperties().limits.framebufferColorSampleCounts;
        VkSampleCountFlags deviceDepthSamples = _physicalDevice->getProperties().limits.framebufferDepthSampleCounts;
        VkSampleCountFlags satisfied = deviceColorSamples & deviceDepthSamples & _traits->samples;
        if (satisfied != 0)
        {
            uint32_t highest = 1 << static_cast<uint32_t>(floor(log2(satisfied)));
            _framebufferSamples = static_cast<VkSampleCountFlagBits>(highest);
        }
        else
        {
            _framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
        }
    }
    else
    {
        _framebufferSamples = VK_SAMPLE_COUNT_1_BIT;
    }
}

void Window::_initDevice()
{
    // if required set up physical device
    if (!_physicalDevice)
    {
        _initPhysicalDevice();
    }

    // set up logical device
    const vsg::Names& validatedNames = _traits->requestedLayers;

    vsg::Names deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.insert(deviceExtensions.end(), _traits->deviceExtensionNames.begin(), _traits->deviceExtensionNames.end());

    auto [graphicsFamily, presentFamily] = _physicalDevice->getQueueFamily(_traits->queueFlags, _surface);
    if (graphicsFamily < 0 || presentFamily < 0) throw Exception{"Error: vsg::Window::create(...) failed to create Window, no suitable Vulkan Device available.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

    vsg::QueueSettings queueSettings{vsg::QueueSetting{graphicsFamily, _traits->queuePiorities}, vsg::QueueSetting{presentFamily, {1.0}}};
    _device = vsg::Device::create(_physicalDevice, queueSettings, validatedNames, deviceExtensions, _traits->deviceFeatures, _instance->getAllocationCallbacks());

    _initFormats();
}

void Window::_initRenderPass()
{
    if (!_device) _initDevice();

    bool requiresDepthRead = (_traits->depthImageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;

    if (_framebufferSamples == VK_SAMPLE_COUNT_1_BIT)
    {
        _renderPass = vsg::createRenderPass(_device, _imageFormat.format, _depthFormat, requiresDepthRead);
    }
    else
    {
        _renderPass = vsg::createMultisampledRenderPass(_device, _imageFormat.format, _depthFormat, _framebufferSamples, requiresDepthRead);
    }
}

void Window::_initSwapchain()
{
    if (!_device) _initDevice();
    if (!_renderPass) _initRenderPass();

    buildSwapchain();
}

void Window::buildSwapchain()
{
    if (_swapchain)
    {
        // make sure all operations on the device have stopped before we go on deleting associated resources
        vkDeviceWaitIdle(*_device);

        // clean up previous swap chain before we begin creating a new one.
        _frames.clear();
        _indices.clear();

        _depthImageView.reset();
        _depthImage.reset();

        _multisampleImage.reset();
        _multisampleImageView.reset();
    }

    // is width and height even required here as the surface appears to control it?
    _swapchain = Swapchain::create(_physicalDevice, _device, _surface, _extent2D.width, _extent2D.height, _traits->swapchainPreferences, _swapchain);

    // pass back the extents used by the swap chain.
    _extent2D = _swapchain->getExtent();

    bool multisampling = _framebufferSamples != VK_SAMPLE_COUNT_1_BIT;
    if (multisampling)
    {
        _multisampleImage = Image::create();
        _multisampleImage->imageType = VK_IMAGE_TYPE_2D;
        _multisampleImage->format = _imageFormat.format;
        _multisampleImage->extent.width = _extent2D.width;
        _multisampleImage->extent.height = _extent2D.height;
        _multisampleImage->extent.depth = 1;
        _multisampleImage->mipLevels = 1;
        _multisampleImage->arrayLayers = 1;
        _multisampleImage->samples = _framebufferSamples;
        _multisampleImage->tiling = VK_IMAGE_TILING_OPTIMAL;
        _multisampleImage->usage = _traits->swapchainPreferences.imageUsage;
        _multisampleImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        _multisampleImage->flags = 0;
        _multisampleImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        _multisampleImage->compile(_device);
        _multisampleImage->allocateAndBindMemory(_device);

        _multisampleImageView = ImageView::create(_multisampleImage, VK_IMAGE_ASPECT_COLOR_BIT);
        _multisampleImageView->compile(_device);
    }

    bool requiresDepthRead = (_traits->depthImageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;
    bool requiresDepthResolve = (multisampling && requiresDepthRead);

    // create depth buffer
    _depthImage = Image::create();
    _depthImage->imageType = VK_IMAGE_TYPE_2D;
    _depthImage->extent.width = _extent2D.width;
    _depthImage->extent.height = _extent2D.height;
    _depthImage->extent.depth = 1;
    _depthImage->mipLevels = 1;
    _depthImage->arrayLayers = 1;
    _depthImage->format = _depthFormat;
    _depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
    _depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _depthImage->samples = _framebufferSamples;
    _depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _depthImage->usage = _traits->depthImageUsage;

    _depthImage->compile(_device);
    _depthImage->allocateAndBindMemory(_device);

    _depthImageView = ImageView::create(_depthImage);
    _depthImageView->compile(_device);

    if (requiresDepthResolve)
    {
        _multisampleDepthImage = _depthImage;
        _multisampleDepthImageView = _depthImageView;

        // create depth buffer
        _depthImage = Image::create();
        _depthImage->imageType = VK_IMAGE_TYPE_2D;
        _depthImage->extent.width = _extent2D.width;
        _depthImage->extent.height = _extent2D.height;
        _depthImage->extent.depth = 1;
        _depthImage->mipLevels = 1;
        _depthImage->arrayLayers = 1;
        _depthImage->format = _depthFormat;
        _depthImage->tiling = VK_IMAGE_TILING_OPTIMAL;
        _depthImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        _depthImage->usage = _traits->depthImageUsage;
        _depthImage->samples = VK_SAMPLE_COUNT_1_BIT;
        _depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        _depthImage->compile(_device);
        _depthImage->allocateAndBindMemory(_device);

        _depthImageView = ImageView::create(_depthImage);
        _depthImageView->compile(_device);
    }

    int graphicsFamily = -1;
    std::tie(graphicsFamily, std::ignore) = _physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT, _surface);

    // set up framebuffer and associated resources
    auto& imageViews = _swapchain->getImageViews();

    _availableSemaphore = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

    size_t initial_indexValue = imageViews.size();
    for (size_t i = 0; i < imageViews.size(); ++i)
    {
        vsg::ImageViews attachments;
        if (_multisampleImageView)
        {
            attachments.push_back(_multisampleImageView);
        }
        attachments.push_back(imageViews[i]);

        if (_multisampleDepthImageView)
        {
            attachments.push_back(_multisampleDepthImageView);
        }
        attachments.push_back(_depthImageView);

        ref_ptr<Framebuffer> fb = Framebuffer::create(_renderPass, attachments, _extent2D.width, _extent2D.height, 1);

        ref_ptr<Semaphore> ias = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

        //_frames.push_back({multisampling ? _multisampleImageView : imageViews[i], fb, ias});
        _frames.push_back({imageViews[i], fb, ias});
        _indices.push_back(initial_indexValue);
    }

    {
        // ensure image attachments are setup on GPU.
        auto commandPool = CommandPool::create(_device, graphicsFamily);
        submitCommandsToQueue(commandPool, _device->getQueue(graphicsFamily), [&](CommandBuffer& commandBuffer) {
            auto depthImageBarrier = ImageMemoryBarrier::create(
                0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                _depthImage,
                _depthImageView->subresourceRange);

            auto pipelineBarrier = PipelineBarrier::create(
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                0, depthImageBarrier);
            pipelineBarrier->record(commandBuffer);

            if (multisampling)
            {
                auto msImageBarrier = ImageMemoryBarrier::create(
                    0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                    _multisampleImage,
                    VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
                auto msPipelineBarrier = PipelineBarrier::create(
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    0, msImageBarrier);
                msPipelineBarrier->record(commandBuffer);
            }
        });
    }
}

VkResult Window::acquireNextImage(uint64_t timeout)
{
    if (!_swapchain) _initSwapchain();

    if (!_availableSemaphore) _availableSemaphore = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

    // check the dimensions of the swapchain and window extents are consistent, if not return a VK_ERROR_OUT_OF_DATE_KHR
    if (_swapchain->getExtent() != _extent2D) return VK_ERROR_OUT_OF_DATE_KHR;

    uint32_t nextImageIndex;
    VkResult result = _swapchain->acquireNextImage(timeout, _availableSemaphore, {}, nextImageIndex);

    if (result == VK_SUCCESS)
    {
        // the acquired image's semaphore must be available now so make it the new _availableSemaphore and set its entry to the one to use for the next frame by swapping ref_ptr<>'s
        _availableSemaphore.swap(_frames[nextImageIndex].imageAvailableSemaphore);

        // shift up previous frame indices
        for (size_t i = _indices.size() - 1; i > 0; --i)
        {
            _indices[i] = _indices[i - 1];
        }

        // update head of _indices to the new frames nextImageIndex
        _indices[0] = nextImageIndex;
    }
    else
    {
        vsg::debug("Window::acquireNextImage(uint64_t timeout) _swapchain->acquireNextImage(...) failed with result = ", result);
    }

    return result;
}

bool Window::pollEvents(vsg::UIEvents& events)
{
    if (!bufferedEvents.empty())
    {
        events.splice(events.end(), bufferedEvents);
        bufferedEvents.clear();
        return true;
    }

    return false;
}
