#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/utils/CommandLine.h>

#include "Draw.h"

#include <iostream>
#include <algorithm>
#include <mutex>
#include <set>

/////////////////////////////////////////////////////////////////////
//
// start of vulkan code
//
namespace vsg
{

using Names = std::vector<const char*>;

Names validateInstancelayerNames(const Names& names)
{
    if (names.empty()) return names;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    using NameSet = std::set<std::string>;
    NameSet layerNames;
    for (const auto& layer : availableLayers)
    {
        std::cout<<"layer="<<layer.layerName<<std::endl;
        if (layer.layerName) layerNames.insert(layer.layerName);
    }

    Names validatedNames;
    validatedNames.reserve(names.size());
    for (const auto& requestedName : names)
    {
        if (layerNames.count(requestedName)!=0)
        {
            std::cout<<"valid requested layer : "<<requestedName<<std::endl;
            validatedNames.push_back(requestedName);
        }
        else
        {
            std::cout<<"Warning : requested invalid layer : "<<requestedName<<std::endl;
        }
    }

    return validatedNames;
}

VkInstance createInstance(Names& instanceExtensions, Names& layers)
{
    VkInstance instance = VK_NULL_HANDLE;

    // applictin info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test";
    appInfo.pEngineName = "VulkanSceneGraph";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.empty() ? nullptr : instanceExtensions.data();

    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        std::cout<<"Failed to create VkInstance"<<std::endl;
        return instance;
    }

    std::cout<<"Created VkInstance"<<std::endl;
    return instance;
}

struct PhysicalDeviceSettings
{
    VkPhysicalDevice device = VK_NULL_HANDLE;
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool complete() const
    {
        return device!=VK_NULL_HANDLE && graphicsFamily>=0 && presentFamily>=0;
    }
};

PhysicalDeviceSettings selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        PhysicalDeviceSettings deviceSettings;
        deviceSettings.device = device;

        // Checked the DeviceQueueFamilyProperties for support for graphics
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamiles(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiles.data());

        for (int i=0; i<queueFamilyCount; ++i)
        {
            const auto& queueFamily = queueFamiles[i];
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)!=0)
            {
                deviceSettings.graphicsFamily = i;
            }

            VkBool32 presentSupported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);
            if (queueFamily.queueCount>0 && presentSupported)
            {
                deviceSettings.presentFamily = i;
            }

            if (deviceSettings.complete()) return deviceSettings;
        }

    }

    return PhysicalDeviceSettings();
}

VkDevice createLogicalDevice(const PhysicalDeviceSettings& deviceSettings, Names& layers, Names& deviceExtensions)
{
    VkDevice device = VK_NULL_HANDLE;

    std::cout<<"createLogicalDevice("<<deviceSettings.device<<", "<<deviceSettings.graphicsFamily<<", "<<deviceSettings.presentFamily<<")"<<std::endl;

    std::set<int> uniqueQueueFamiles = {deviceSettings.graphicsFamily, deviceSettings.presentFamily};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamiles)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.empty() ? nullptr : queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();

    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    if (vkCreateDevice(deviceSettings.device, &createInfo, nullptr, &device)!=VK_SUCCESS)
    {
        std::cout<<"Failed to create logical device"<<std::endl;
    }

    return device;
}

VkQueue createDeviceQueue(VkDevice device, int graphicsFamily)
{
    VkQueue queue;
    vkGetDeviceQueue(device, graphicsFamily, 0, &queue);
    return queue;
}


struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

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

    for (const auto& availablePresentMode : details.presentModes)
    {
        if (availablePresentMode==VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
        else if (availablePresentMode==VK_PRESENT_MODE_IMMEDIATE_KHR) presentMode = availablePresentMode;
    }

    return presentMode;
}


struct SwapChain
{
    VkSwapchainKHR              swapchain = VK_NULL_HANDLE;
    VkFormat                    format;
    VkExtent2D                  extent;
    std::vector<VkImage>        images;
    std::vector<VkImageView>    views;

    bool complete() const { return swapchain!=VK_NULL_HANDLE; }

};

SwapChain createSwapChain(const PhysicalDeviceSettings& deviceSettings, VkDevice logicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height)
{
    SwapChainSupportDetails details = querySwapChainSupport(deviceSettings.device, surface);

    VkSurfaceFormatKHR surfaceFormat = selectSwapSurfaceFormat(details);
    VkPresentModeKHR presentMode = selectSwapPresentMode(details);
    VkExtent2D extent = selectSwapExtent(details, width, height);


    uint32_t imageCount = details.capabilities.minImageCount+1;
    if (details.capabilities.maxImageCount>0 && imageCount>details.capabilities.maxImageCount)
    {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (deviceSettings.graphicsFamily!=deviceSettings.presentFamily)
    {
        uint32_t queueFamilyIndices[] = { uint32_t(deviceSettings.graphicsFamily), uint32_t(deviceSettings.presentFamily) };
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

    SwapChain swapChain;
    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain.swapchain)!=VK_SUCCESS)
    {
        return swapChain;
    }

    vkGetSwapchainImagesKHR(logicalDevice, swapChain.swapchain, &imageCount, nullptr);
    swapChain.images.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain.swapchain, &imageCount, swapChain.images.data());

    swapChain.format = surfaceFormat.format;
    swapChain.extent = extent;

    swapChain.views.resize(swapChain.images.size());
    for (std::size_t i=0; i<swapChain.images.size(); ++i)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChain.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChain.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChain.views[i])!=VK_SUCCESS)
        {
            std::cout<<"Error : unable to create image view "<<i<<std::endl;
        }
    }

    return swapChain;
}

}

//
// end of vulkan code
//
/////////////////////////////////////////////////////////////////////

vsg::Names getInstanceExtensions()
{
    uint32_t glfw_count;
    const char** glfw_extensons = glfwGetRequiredInstanceExtensions(&glfw_count);
    return vsg::Names(glfw_extensons, glfw_extensons+glfw_count);
}

template<typename T>
void print(std::ostream& out, const std::string& description, const T& names)
{
    out<<description<<".size()= "<<names.size()<<std::endl;
    for (const auto& name : names)
    {
        out<<"    "<<name<<std::endl;
    }
}


int main(int argc, char** argv)
{
    bool debugLayer = false;
    uint32_t width = 800;
    uint32_t height = 600;

    try
    {
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--debug","-d"))) debugLayer = true;
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--window","-w"), width, height)) {}
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    // initialize window
    glfwInit();


    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    /////////////////////////////////////////////////////////////////////
    //
    // start of initialize vulkan
    //

    vsg::Names instanceExtensions = getInstanceExtensions();

    vsg::Names requestedLayers;
    if (debugLayer)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);

    vsg::Names deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);


    print(std::cout,"instanceExtensions",instanceExtensions);
    print(std::cout,"validatedNames",validatedNames);

    VkInstance instance = vsg::createInstance(instanceExtensions, validatedNames);
    if (!instance)
    {
        std::cout<<"No VkInstance available!"<<std::endl;
        return 1;
    }

    // use GLFW to create surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        std::cout<<"Failed to create window surface"<<std::endl;
        return 1;
    }

    // set up device
    vsg::PhysicalDeviceSettings physicalDeviceSettings = vsg::selectPhysicalDevice(instance, surface);
    if (!physicalDeviceSettings.complete())
    {
        std::cout<<"No VkPhysicalDevice available!"<<std::endl;
        return 1;
    }

    // set up logical device
    VkDevice device = vsg::createLogicalDevice(physicalDeviceSettings, validatedNames, deviceExtensions);
    if (!device)
    {
        std::cout<<"No VkDevice available!"<<std::endl;
        return 1;
    }

    std::cout<<"Created logical device "<<device<<std::endl;

    VkQueue graphicsQueue = vsg::createDeviceQueue(device, physicalDeviceSettings.graphicsFamily);
    if (!graphicsQueue)
    {
        std::cout<<"No Graphics queue available!"<<std::endl;
        return 1;
    }

    VkQueue presentQueue = vsg::createDeviceQueue(device, physicalDeviceSettings.presentFamily);
    if (!presentQueue)
    {
        std::cout<<"No Present queue available!"<<std::endl;
        return 1;
    }

    std::cout<<"Created graphicsQueue="<<graphicsQueue<<", presentQueue="<<presentQueue<<std::endl;


    vsg::SwapChain swapChain = vsg::createSwapChain(physicalDeviceSettings, device, surface, width, height);
    if (!swapChain.complete())
    {
        std::cout<<"Failed to create swap chain"<<std::endl;
        return 1;
    }

    std::cout<<"Created swapchain with "<<swapChain.images.size()<<", "<<swapChain.views.size()<<std::endl;

        //
    // end of initialize vulkan
    //
    /////////////////////////////////////////////////////////////////////


    // main loop
    while(!glfwWindowShouldClose(window))
    {
        //std::cout<<"In main loop"<<std::endl;
        glfwPollEvents();
    }

    // clean up vulkan
    for(auto imageView : swapChain.views)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain.swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    // clean up GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}