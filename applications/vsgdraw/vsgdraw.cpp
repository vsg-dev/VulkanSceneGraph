#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vsg/core/ref_ptr.h>

#include "Draw.h"

#include <iostream>
#include <algorithm>
#include <mutex>
#include <set>

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

bool createInstance(VkInstance& instance, Names& instanceExtensions, Names& layers)
{
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
        return false;
    }

    std::cout<<"Created VkInstance"<<std::endl;
    return true;
}

Names getInstanceExtensions()
{
    uint32_t glfw_count;
    const char** glfw_extensons = glfwGetRequiredInstanceExtensions(&glfw_count);
    return Names(glfw_extensons, glfw_extensons+glfw_count);
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
    bool debugLayer = true;

    int width = 800;
    int height = 600;

    // initialize window
    glfwInit();

    Names instanceExtensions = getInstanceExtensions();

    Names requestedLayers;
    if (debugLayer)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    Names validatedNames = validateInstancelayerNames(requestedLayers);

    print(std::cout,"instanceExtensions",instanceExtensions);
    print(std::cout,"validatedNames",validatedNames);

    // initialize vulkan
    VkInstance instance;

    if (!createInstance(instance, instanceExtensions, validatedNames))
    {
        std::cout<<"No VkInstance no play!"<<std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);


    // main loop
    while(!glfwWindowShouldClose(window))
    {
        //std::cout<<"In main loop"<<std::endl;
        glfwPollEvents();
    }

    // clean up
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}