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
    for(const auto& layer : availableLayers)
    {
        std::cout<<"layer="<<layer.layerName<<std::endl;
        if (layer.layerName) layerNames.insert(layer.layerName);
    }

    Names validatedNames;
    validatedNames.reserve(names.size());
    for(const auto& requestedName : names)
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

    for(const auto& device : devices)
    {
        PhysicalDeviceSettings deviceSettings;
        deviceSettings.device = device;

        // Checked the DeviceQueueFamilyProperties for support for graphics
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamiles(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiles.data());

        for(int i=0; i<queueFamilyCount; ++i)
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

VkDevice createLogicalDevice(const PhysicalDeviceSettings& deviceSettings, Names& layers)
{
    VkDevice device = VK_NULL_HANDLE;

    std::cout<<"createLogicalDevice("<<deviceSettings.device<<", "<<deviceSettings.graphicsFamily<<", "<<deviceSettings.presentFamily<<")"<<std::endl;

    std::set<int> uniqueQueueFamiles = {deviceSettings.graphicsFamily, deviceSettings.presentFamily};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for(int queueFamily : uniqueQueueFamiles)
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
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
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
    for(const auto& name : names)
    {
        out<<"    "<<name<<std::endl;
    }
}


int main(int argc, char** argv)
{
    bool debugLayer = false;
    int width = 800;
    int height = 600;

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
    VkDevice device = vsg::createLogicalDevice(physicalDeviceSettings, validatedNames);
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
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    // clean up GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}