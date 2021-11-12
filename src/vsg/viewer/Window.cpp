/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/PipelineBarrier.h>
#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/viewer/Window.h>
#include <vsg/vk/SubmitCommands.h>

#include <array>
#include <chrono>

using namespace vsg;

Window::Window(ref_ptr<WindowTraits> traits) :
    _traits(traits),
    _extent2D{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()},
    _clearColor{{0.2f, 0.2f, 0.4f, 1.0f}},
    _framebufferSamples(VK_SAMPLE_COUNT_1_BIT)
{
}

Window::~Window()
{
    // do we need to call clear()?
}

void Window::clear()
{
    _frames.clear();
    _swapchain = 0;

    _depthImage = 0;
    _depthImageMemory = 0;
    _depthImageView = 0;

    _renderPass = 0;
    _surface = 0;
    _device = 0;
    _physicalDevice = 0;
}

void Window::share(Window& window)
{
    _instance = window.getOrCreateInstance();
    _physicalDevice = window.getOrCreatePhysicalDevice();
    _device = window.getOrCreateDevice();
    _renderPass = window.getOrCreateRenderPass();

    _initSurface();
    _initFormats();
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
    if (!_physicalDevice) _initDevice();
    return _physicalDevice;
}

void Window::setDevice(ref_ptr<Device> device)
{
    _device = device;
    if (_device)
    {
        _physicalDevice = _device->getPhysicalDevice();
        _initFormats();
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
    vsg::Names instanceExtensions = _traits->instanceExtensionNames;

    instanceExtensions.push_back("VK_KHR_surface");
    instanceExtensions.push_back(instanceExtensionSurfaceName());

    vsg::Names requestedLayers;
    if (_traits->debugLayer || _traits->apiDumpLayer)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        requestedLayers.push_back("VK_LAYER_KHRONOS_validation");         // new validation layer name
        requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation"); // old validation layer name
        if (_traits->apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
    }

    vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);
    _instance = vsg::Instance::create(instanceExtensions, validatedNames, _traits->vulkanVersion);
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
    if (!_instance) _initInstance();
    if (!_surface) _initSurface();

    // if required set up physical device
    if (!_physicalDevice)
    {
        _physicalDevice = _instance->getPhysicalDevice(_traits->queueFlags, _surface, _traits->deviceTypePreferences);
        if (!_physicalDevice) throw Exception{"Error: vsg::Window::create(...) failed to create Window,  no suitable Vulkan PhysicalDevice available.", VK_ERROR_INVALID_EXTERNAL_HANDLE};
    }

    // set up logical device
    vsg::Names requestedLayers;
    if (_traits->debugLayer)
    {
        requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        if (_traits->apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
    }

    vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);

    vsg::Names deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.insert(deviceExtensions.end(), _traits->deviceExtensionNames.begin(), _traits->deviceExtensionNames.end());

    auto [graphicsFamily, presentFamily] = _physicalDevice->getQueueFamily(_traits->queueFlags, _surface);
    if (graphicsFamily < 0 || presentFamily < 0) throw Exception{"Error: vsg::Window::create(...) failed to create Window, no suitable Vulkan Device available.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

    vsg::QueueSettings queueSettings{vsg::QueueSetting{graphicsFamily, {1.0}}, vsg::QueueSetting{presentFamily, {1.0}}};
    _device = vsg::Device::create(_physicalDevice, queueSettings, validatedNames, deviceExtensions, _traits->deviceFeatures, _instance->getAllocationCallbacks());

    _initFormats();
}

void Window::_initRenderPass()
{
    if (!_device) _initDevice();

    if (_framebufferSamples == VK_SAMPLE_COUNT_1_BIT)
    {
        _renderPass = vsg::createRenderPass(_device, _imageFormat.format, _depthFormat);
    }
    else
    {
        _renderPass = vsg::createMultisampledRenderPass(_device, _imageFormat.format, _depthFormat, _framebufferSamples);
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
        // make sure all operations on the device have stopped before we go deleting associated resources
        vkDeviceWaitIdle(*_device);

        // clean up previous swap chain before we begin creating a new one.
        _frames.clear();
        _indices.clear();

        _depthImageView = 0;
        _depthImage = 0;
        _depthImageMemory = 0;

        _multisampleImage = 0;
        _multisampleImageView = 0;

        _swapchain = 0;
    }

    // is width and height even required here as the surface appear to control it.
    _swapchain = Swapchain::create(_physicalDevice, _device, _surface, _extent2D.width, _extent2D.height, _traits->swapchainPreferences);

    // pass back the extents used by the swap chain.
    _extent2D = _swapchain->getExtent();

    auto deviceID = _device->deviceID;

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
        _multisampleImage->usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        _multisampleImage->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        _multisampleImage->flags = 0;
        _multisampleImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        _multisampleImage->compile(_device);

        auto colorMemory = DeviceMemory::create(_device, _multisampleImage->getMemoryRequirements(deviceID), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        _multisampleImage->bind(colorMemory, 0);

        _multisampleImageView = ImageView::create(_multisampleImage, VK_IMAGE_ASPECT_COLOR_BIT);
        _multisampleImageView->compile(_device);
    }

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
    _depthImage->samples = _framebufferSamples;
    _depthImage->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    _depthImage->compile(_device);

    _depthImageMemory = DeviceMemory::create(_device, _depthImage->getMemoryRequirements(deviceID), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _depthImage->bind(_depthImageMemory, 0);

    _depthImageView = ImageView::create(_depthImage);
    _depthImageView->compile(_device);

    int graphicsFamily = -1;
    std::tie(graphicsFamily, std::ignore) = _physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT, _surface);

    // set up framebuffer and associated resources
    auto& imageViews = _swapchain->getImageViews();

    _availableSemaphore = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

    size_t initial_indexValue = imageViews.size();
    for (size_t i = 0; i < imageViews.size(); ++i)
    {
        vsg::ImageViews attachments;
        if (multisampling)
        {
            attachments.push_back(_multisampleImageView);
        }
        attachments.push_back(imageViews[i]);
        attachments.push_back(_depthImageView);

        ref_ptr<Framebuffer> fb = Framebuffer::create(_renderPass, attachments, _extent2D.width, _extent2D.height, 1);

        ref_ptr<Semaphore> ias = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);

        _frames.push_back({multisampling ? _multisampleImageView : imageViews[i], fb, ias});
        _indices.push_back(initial_indexValue);
    }

    {
        // ensure image attachments are setup on GPU.
        ref_ptr<CommandPool> commandPool = CommandPool::create(_device, graphicsFamily);
        submitCommandsToQueue(_device, commandPool, _device->getQueue(graphicsFamily), [&](CommandBuffer& commandBuffer) {
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

    uint32_t imageIndex;
    VkResult result = _swapchain->acquireNextImage(timeout, _availableSemaphore, {}, imageIndex);

    if (result == VK_SUCCESS)
    {
        // the acquired image's semaphore must be available now so make it the new _availableSemaphore and set it's entry to the one to use of the next frame by swapping ref_ptr<>'s
        _availableSemaphore.swap(_frames[imageIndex].imageAvailableSemaphore);

        // shift up previous frame indices
        for (size_t i = 1; i < _indices.size(); ++i)
        {
            _indices[i] = _indices[i - 1];
        }

        // update head of _indices to the new frames imageIndex
        _indices[0] = imageIndex;
    }
    else
    {
        // TODO: Need to think about what should happen on failure
    }

    return result;
}

bool Window::pollEvents(vsg::UIEvents& events)
{
    if (bufferedEvents.size() > 0)
    {
        events.splice(events.end(), bufferedEvents);
        bufferedEvents.clear();
        return true;
    }

    return false;
}
