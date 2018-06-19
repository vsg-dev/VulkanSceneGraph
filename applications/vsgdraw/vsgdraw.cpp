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

using PhysialDeviceQueueFamilyPair = std::pair<VkPhysicalDevice, int>;

PhysialDeviceQueueFamilyPair selectPhysicalDevice(VkInstance& instance, int queueRequirementsMask=VK_QUEUE_GRAPHICS_BIT)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
        // Checked the DeviceQueueFamilyProperties for support for graphics
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamiles(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiles.data());

        for(int i=0; i<queueFamilyCount; ++i)
        {
            const auto& queueFamily = queueFamiles[i];
            if ((queueFamily.queueFlags & queueRequirementsMask)!=0)
            {
                return PhysialDeviceQueueFamilyPair(device, i);
            }
        }

    }

    return PhysialDeviceQueueFamilyPair(VK_NULL_HANDLE,0);
}

VkDevice createLogicalDevice(const PhysialDeviceQueueFamilyPair& physicalDevicePair, Names& layers)
{
    VkDevice device = VK_NULL_HANDLE;

    std::cout<<"createLogicalDevice("<<physicalDevicePair.first<<", "<<physicalDevicePair.second<<")"<<std::endl;

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = physicalDevicePair.second;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    if (vkCreateDevice(physicalDevicePair.first, &createInfo, nullptr, &device)!=VK_SUCCESS)
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

    // set up device
    vsg::PhysialDeviceQueueFamilyPair physicalDevicePair = vsg::selectPhysicalDevice(instance, VK_QUEUE_GRAPHICS_BIT);
    if (!physicalDevicePair.first)
    {
        std::cout<<"No VkPhysicalDevice available!"<<std::endl;
        return 1;
    }

    // set up logical device
    VkDevice device = vsg::createLogicalDevice(physicalDevicePair, validatedNames);
    if (!device)
    {
        std::cout<<"No VkDevice available!"<<std::endl;
        return 1;
    }

    std::cout<<"Created logical device "<<device<<std::endl;

    VkQueue queue = vsg::createDeviceQueue(device, physicalDevicePair.second);
    if (!queue)
    {
        std::cout<<"No VkQeue available!"<<std::endl;
        return 1;
    }

    //
    // end of initialize vulkan
    //
    /////////////////////////////////////////////////////////////////////

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);


    // main loop
    while(!glfwWindowShouldClose(window))
    {
        //std::cout<<"In main loop"<<std::endl;
        glfwPollEvents();
    }

    // clean up vulkan
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

    // clean up GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}