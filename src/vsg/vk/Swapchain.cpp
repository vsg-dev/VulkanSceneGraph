/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/vk/Device.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>

#include <algorithm>
#include <limits>

using namespace vsg;

SwapChainSupportDetails vsg::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    details.formats.resize(formatCount);
    if (formatCount > 0)
    {
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    details.presentModes.resize(presentModeCount);
    if (presentModeCount > 0)
    {
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR vsg::selectSwapSurfaceFormat(const SwapChainSupportDetails& details, VkSurfaceFormatKHR preferredSurfaceFormat)
{
    if (details.formats.empty() || (details.formats.size() == 1 && details.formats[0].format == VK_FORMAT_UNDEFINED))
    {
        warn("selectSwapSurfaceFormat() VK_FORMAT_UNDEFINED, so using fallback ");
        return {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // check if requested format is available
    for (const auto& availableFormat : details.formats)
    {
        if (availableFormat.format == preferredSurfaceFormat.format && availableFormat.colorSpace == preferredSurfaceFormat.colorSpace)
        {
            return availableFormat;
        }
    }

    // fallback to checking for {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
    for (const auto& availableFormat : details.formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // fallback to using the first on the list of available formats
    return details.formats[0];
}

VkExtent2D vsg::selectSwapExtent(const SwapChainSupportDetails& details, uint32_t width, uint32_t height)
{
    const VkSurfaceCapabilitiesKHR& capabilities = details.capabilities;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D extent;
        extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
        extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));
        return extent;
    }
}

VkPresentModeKHR vsg::selectSwapPresentMode(const SwapChainSupportDetails& details, VkPresentModeKHR preferredPresentMode)
{
    // select requested presentMode if it's available.
    for (auto availablePresentMode : details.presentModes)
    {
        if (availablePresentMode == preferredPresentMode) return availablePresentMode;
    }

    // requested presentMode not available so fallback to checking if VK_PRESENT_MODE_MAILBOX_KHR available
    for (auto availablePresentMode : details.presentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
    }

    // fallback to VK_PRESENT_MODE_FIFO_KHR
    return VK_PRESENT_MODE_FIFO_KHR;

    /**
    From https://github.com/LunarG/VulkanSamples/issues/98 :

    VK_PRESENT_MODE_IMMEDIATE_KHR. This is for applications that don't care about tearing, or have some way of synchronizing with the display (which Vulkan doesn't yet provide).

    VK_PRESENT_MODE_FIFO_KHR. This is for applications that don't want tearing ever. It's difficult to say how fast they may be, whether they care about stuttering/latency.

    VK_PRESENT_MODE_FIFO_RELAXED_KHR. This is for applications that generally render/present a new frame every refresh cycle, but are occasionally late. In this case (perhaps because of stuttering/latency concerns), they want the late image to be immediately displayed, even though that may mean some tearing.

    VK_PRESENT_MODE_MAILBOX_KHR. I'm guessing that this is for applications that generally render/present a new frame every refresh cycle, but are occasionally early. In this case, they want the new image to be displayed instead of the previously-queued-for-presentation image that has not yet been displayed.
    **/
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// SwapchainImage
//
namespace vsg
{
    // helper class that disables the automatic clean up of the swap chain image as the swap chain itself manages its lifetime
    class SwapchainImage : public Inherit<Image, SwapchainImage>
    {
    public:
        SwapchainImage(VkImage image, Device* device) :
            Inherit(image, device)
        {
        }

    protected:
        virtual ~SwapchainImage()
        {
            for (auto& vd : _vulkanData)
            {
                vd.deviceMemory = nullptr;
                vd.image = VK_NULL_HANDLE;
            }
        }
    };
    VSG_type_name(vsg::SwapchainImage);

} // namespace vsg

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Swapchain
//
Swapchain::Swapchain(PhysicalDevice* physicalDevice, Device* device, Surface* surface, uint32_t width, uint32_t height, SwapchainPreferences& preferences, ref_ptr<Swapchain> oldSwapchain) :
    _device(device)
{
    SwapChainSupportDetails details = querySwapChainSupport(*physicalDevice, *surface);

    VkSurfaceFormatKHR surfaceFormat = selectSwapSurfaceFormat(details, preferences.surfaceFormat);
    VkPresentModeKHR presentMode = selectSwapPresentMode(details, preferences.presentMode);
    VkExtent2D extent = selectSwapExtent(details, width, height);

    uint32_t imageCount = std::max(preferences.imageCount, details.capabilities.minImageCount);                        // Vulkan spec requires minImageCount to be 1 or greater
    if (details.capabilities.maxImageCount > 0) imageCount = std::min(imageCount, details.capabilities.maxImageCount); // Vulkan spec specifies 0 as being unlimited number of images

    // apply the selected settings back to preferences so calling code can determine the active settings.
    preferences.imageCount = imageCount;
    preferences.presentMode = presentMode;
    preferences.surfaceFormat = surfaceFormat;

    debug("Swapchain::create(...., width = ", width, ", height = ", height, ")");
    debug("     details.capabilities.minImageCount=", details.capabilities.minImageCount);
    debug("     details.capabilities.maxImageCount=", details.capabilities.maxImageCount);
    debug("     imageCount = ", imageCount);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = *surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = preferences.imageUsage;

    auto [graphicsFamily, presentFamily] = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT, surface);
    if (graphicsFamily != presentFamily)
    {
        uint32_t queueFamilyIndices[] = {uint32_t(graphicsFamily), uint32_t(presentFamily)};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (oldSwapchain)
    {
        createInfo.oldSwapchain = *(oldSwapchain);
    }

    createInfo.pNext = nullptr;

    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(*device, &createInfo, _device->getAllocationCallbacks(), &swapchain);
    if (result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create swap chain.", result};
    }

    // assign data to this Swapchain object
    _surface = surface;
    _swapchain = swapchain;

    _format = surfaceFormat.format;
    _extent = extent;

    // create the ImageViews
    vkGetSwapchainImagesKHR(*device, swapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(*device, swapchain, &imageCount, images.data());

    for (std::size_t i = 0; i < images.size(); ++i)
    {
        auto imageView = ImageView::create(SwapchainImage::create(images[i], device));
        imageView->viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageView->format = surfaceFormat.format;
        imageView->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageView->subresourceRange.baseMipLevel = 0;
        imageView->subresourceRange.levelCount = 1;
        imageView->subresourceRange.baseArrayLayer = 0;
        imageView->subresourceRange.layerCount = 1;
        imageView->compile(device);

        _imageViews.push_back(imageView);
    }
}

Swapchain::~Swapchain()
{
    _imageViews.clear();

    if (_swapchain)
    {
        debug("Calling vkDestroySwapchainKHR(..)");
        vkDestroySwapchainKHR(*_device, _swapchain, _device->getAllocationCallbacks());
    }
}

VkResult Swapchain::acquireNextImage(uint64_t timeout, ref_ptr<Semaphore> semaphore, ref_ptr<Fence> fence, uint32_t& imageIndex)
{
    VkSemaphore vk_semaphore = semaphore ? semaphore->vk() : VK_NULL_HANDLE;
    VkFence vk_fence = fence ? fence->vk() : VK_NULL_HANDLE;
    return vkAcquireNextImageKHR(*_device, _swapchain, timeout, vk_semaphore, vk_fence, &imageIndex);
}
