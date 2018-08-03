#include <vsg/vk/Device.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>

#include <limits>
#include <iostream>

namespace vsg
{

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    details.formats.resize(formatCount);
    if (formatCount>0)
    {
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    details.presentModes.resize(presentModeCount);
    if (presentModeCount>0)
    {
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR selectSwapSurfaceFormat(SwapChainSupportDetails& details)
{
    if (details.formats.empty() || (details.formats.size()==1 && details.formats[0].format==VK_FORMAT_UNDEFINED))
    {
        std::cout<<"selectSwapSurfaceFormat() VK_FORMAT_UNDEFINED, so using fallbalck "<<std::endl;
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto& availableFormat : details.formats)
    {
        if (availableFormat.format==VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return details.formats[0];
}

VkExtent2D selectSwapExtent(SwapChainSupportDetails& details, uint32_t width, uint32_t height)
{
    VkSurfaceCapabilitiesKHR& capabilities = details.capabilities;

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

VkPresentModeKHR selectSwapPresentMode(SwapChainSupportDetails& details)
{
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
#if 0
    for (const auto& availablePresentMode : details.presentModes)
    {
        if (availablePresentMode==VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
        else if (availablePresentMode==VK_PRESENT_MODE_IMMEDIATE_KHR) presentMode = availablePresentMode;
    }
#endif

 /**
    From https://github.com/LunarG/VulkanSamples/issues/98 :

    VK_PRESENT_MODE_IMMEDIATE_KHR. This is for applications that don't care about tearing, or have some way of synchronizing with the display (which Vulkan doesn't yet provide).

    VK_PRESENT_MODE_FIFO_KHR. This is for applications that don't want tearing ever. It's difficult to say how fast they may be, whether they care about stuttering/latency.

    VK_PRESENT_MODE_FIFO_RELAXED_KHR. This is for applications that generally render/present a new frame every refresh cycle, but are occasionally late. In this case (perhaps because of stuttering/latency concerns), they want the late image to be immediately displayed, even though that may mean some tearing.

    VK_PRESENT_MODE_MAILBOX_KHR. I'm guessing that this is for applications that generally render/present a new frame every refresh cycle, but are occasionally early. In this case, they want the new image to be displayed instead of the previously-queued-for-presentation image that has not yet been displayed.
**/

    return presentMode;
}

Swapchain::Swapchain(Device* device, Surface* surface, VkSwapchainKHR swapchain, AllocationCallbacks*  allocator):
    _device(device), _surface(surface), _swapchain(swapchain), _allocator(allocator)
{
}


Swapchain::Result Swapchain::create(PhysicalDevice* physicalDevice, Device* device, Surface* surface, uint32_t width, uint32_t height, AllocationCallbacks*  allocator)
{
    if (!physicalDevice || !device || !surface)
    {
        return Swapchain::Result("Error: vsg::Swapchain::create(...) failed to create swapchain, undefined inputs.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    std::cout<<"Swapchain::create(...., width = "<<width<<", height = "<<height<<")"<<std::endl;

    SwapChainSupportDetails details = querySwapChainSupport(*physicalDevice, *surface);

    VkSurfaceFormatKHR surfaceFormat = selectSwapSurfaceFormat(details);
    VkPresentModeKHR presentMode = selectSwapPresentMode(details);
    VkExtent2D extent = selectSwapExtent(details, width, height);

    uint32_t imageCount = details.capabilities.minImageCount+1;
    if (details.capabilities.maxImageCount>0 && imageCount>details.capabilities.maxImageCount)
    {
        imageCount = details.capabilities.maxImageCount;
    }

    std::cout<<"Swapcain::create(..) "<<std::endl;
    std::cout<<"     details.capabilities.minImageCount="<<details.capabilities.minImageCount<<std::endl;
    std::cout<<"     details.capabilities.maxImageCount="<<details.capabilities.maxImageCount<<std::endl;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = *surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (physicalDevice->getGraphicsFamily()!=physicalDevice->getPresentFamily())
    {
        uint32_t queueFamilyIndices[] = { uint32_t(physicalDevice->getGraphicsFamily()), uint32_t(physicalDevice->getPresentFamily()) };
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

    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(*device, &createInfo, *allocator, &swapchain);
    if (result != VK_SUCCESS)
    {
        return Result("Error: Failed to create swap chain.", result);
    }

    ref_ptr<Swapchain> sw = new Swapchain(device, surface, swapchain);

    sw->_format = surfaceFormat.format;
    sw->_extent = extent;

    // create the ImageViews
    vkGetSwapchainImagesKHR(*device, swapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(*device, swapchain, &imageCount, images.data());

    for (std::size_t i=0; i<images.size(); ++i)
    {
        ref_ptr<ImageView> view = ImageView::create(device, images[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, allocator);
        if (view) sw->getImageViews().push_back(view);
    }

    return sw;
}

Swapchain::~Swapchain()
{
    _imageViews.clear();

    if (_swapchain)
    {
        std::cout<<"Calling vkDestroySwapchainKHR(..)"<<std::endl;
        vkDestroySwapchainKHR(*_device, _swapchain, *_allocator);
    }
}


}
