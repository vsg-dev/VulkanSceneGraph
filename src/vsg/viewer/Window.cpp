/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/PipelineBarrier.h>
#include <vsg/core/Exception.h>
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
    _framebufferSamples(VK_SAMPLE_COUNT_1_BIT),
    _nextImageIndex(0)
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

void Window::share(const Window& window)
{
    _instance = window._instance;
    _physicalDevice = window._physicalDevice;
    _device = window._device;
    _renderPass = window._renderPass;
}

void Window::_initInstance()
{
    if (_traits->device)
    {
        _instance = _traits->device->getInstance();
    }
    else
    {
        // create the vkInstance
        vsg::Names instanceExtensions = _traits->instanceExtensionNames;

        instanceExtensions.push_back("VK_KHR_surface");
        instanceExtensions.push_back(instanceExtensionSurfaceName());

        vsg::Names requestedLayers;
        if (_traits->debugLayer)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
            if (_traits->apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
        }

        // TODO need to decide whether we need to have a Window::_allocator or traits member.
        vsg::AllocationCallbacks* allocator = nullptr;

        vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);
        _instance = vsg::Instance::create(instanceExtensions, validatedNames, allocator);
    }
}

void Window::_initDevice()
{
    if (!_instance) _initInstance();
    if (!_surface) _initSurface();

    // Device
    if (_traits->device)
    {
        _device = _traits->device;
        _physicalDevice = _device->getPhysicalDevice();
    }
    else
    {
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

        // set up device
        auto [physicalDevice, queueFamily, presentFamily] = _instance->getPhysicalDeviceAndQueueFamily(_traits->queueFlags, _surface);
        if (!physicalDevice || queueFamily < 0 || presentFamily < 0) throw Exception{"Error: vsg::Window::create(...) failed to create Window, no Vulkan PhysicalDevice supported.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

        vsg::QueueSettings queueSettings{vsg::QueueSetting{queueFamily, {1.0}}, vsg::QueueSetting{presentFamily, {1.0}}};
        _device = vsg::Device::create(physicalDevice, queueSettings, validatedNames, deviceExtensions, _traits->allocator);
        _physicalDevice = physicalDevice;
    }

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

void Window::_initRenderPass()
{
    if (!_device) _initDevice();

    vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*_physicalDevice, *_surface);

    _imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
    _depthFormat = VK_FORMAT_D24_UNORM_S8_UINT; //VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_SFLOAT_S8_UINT

    if (_framebufferSamples == VK_SAMPLE_COUNT_1_BIT)
    {
        _renderPass = vsg::createRenderPass(_device, _imageFormat.format, _depthFormat, _traits->allocator);
    }
    else
    {
        _renderPass = vsg::createMultisampledRenderPass(_device, _imageFormat.format, _depthFormat, _framebufferSamples, _traits->allocator);
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

    bool multisampling = _framebufferSamples != VK_SAMPLE_COUNT_1_BIT;
    if (multisampling)
    {
        VkImageCreateInfo colorImageCreateInfo = {};
        colorImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        colorImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        colorImageCreateInfo.format = _imageFormat.format;
        colorImageCreateInfo.extent.width = _extent2D.width;
        colorImageCreateInfo.extent.height = _extent2D.height;
        colorImageCreateInfo.extent.depth = 1;
        colorImageCreateInfo.mipLevels = 1;
        colorImageCreateInfo.arrayLayers = 1;
        colorImageCreateInfo.samples = _framebufferSamples;
        colorImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        colorImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        colorImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorImageCreateInfo.flags = 0;
        colorImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        colorImageCreateInfo.queueFamilyIndexCount = 0;
        colorImageCreateInfo.pNext = nullptr;
        _multisampleImage = Image::create(_device, colorImageCreateInfo);

        auto colorMemory = DeviceMemory::create(_device, _multisampleImage->getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        _multisampleImage->bind(colorMemory, 0);

        _multisampleImageView = ImageView::create(_device, _multisampleImage, VK_IMAGE_VIEW_TYPE_2D, _imageFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    // create depth buffer
    VkImageCreateInfo depthImageCreateInfo = {};
    depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageCreateInfo.extent.width = _extent2D.width;
    depthImageCreateInfo.extent.height = _extent2D.height;
    depthImageCreateInfo.extent.depth = 1;
    depthImageCreateInfo.mipLevels = 1;
    depthImageCreateInfo.arrayLayers = 1;
    depthImageCreateInfo.format = _depthFormat;
    depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageCreateInfo.samples = _framebufferSamples;
    depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthImageCreateInfo.pNext = nullptr;

    _depthImage = Image::create(_device, depthImageCreateInfo);

    _depthImageMemory = DeviceMemory::create(_device, _depthImage->getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkBindImageMemory(*_device, *_depthImage, *_depthImageMemory, 0);

    _depthImageView = ImageView::create(_device, _depthImage, VK_IMAGE_VIEW_TYPE_2D, _depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    int graphicsFamily = -1;
    std::tie(graphicsFamily, std::ignore) = _physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT, _surface);

    // set up framebuffer and associated resources
    Swapchain::ImageViews& imageViews = _swapchain->getImageViews();

    for (size_t i = 0; i < imageViews.size(); ++i)
    {
        std::vector<VkImageView> attachments;

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *_renderPass;
        if (multisampling)
        {
            attachments.push_back(*_multisampleImageView);
        }
        attachments.push_back(*imageViews[i]);
        attachments.push_back(*_depthImageView);
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _extent2D.width;
        framebufferInfo.height = _extent2D.height;
        framebufferInfo.layers = 1;

        ref_ptr<Semaphore> ias = vsg::Semaphore::create(_device, _traits->imageAvailableSemaphoreWaitFlag);
        ref_ptr<Framebuffer> fb = Framebuffer::create(_device, framebufferInfo);

        _frames.push_back({multisampling ? _multisampleImageView : imageViews[i], fb, ias});
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
                VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1});

            auto pipelineBarrier = PipelineBarrier::create(
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                0, depthImageBarrier);
            pipelineBarrier->dispatch(commandBuffer);

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
                msPipelineBarrier->dispatch(commandBuffer);
            }
        });
    }

    _nextImageIndex = 0;
}
