/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Instance.h>

#include <iostream>
#include <set>

using namespace vsg;

Names vsg::validateInstancelayerNames(const Names& names)
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
        std::cout << "layer=" << layer.layerName << std::endl;
        if (layer.layerName[0] != 0) layerNames.insert(layer.layerName);
    }

    Names validatedNames;
    validatedNames.reserve(names.size());
    for (const auto& requestedName : names)
    {
        if (layerNames.count(requestedName) != 0)
        {
            std::cout << "valid requested layer : " << requestedName << std::endl;
            validatedNames.push_back(requestedName);
        }
        else
        {
            std::cout << "Warning : requested invalid layer : " << requestedName << std::endl;
        }
    }

    return validatedNames;
}

Instance::Instance(VkInstance instance, AllocationCallbacks* allocator) :
    _instance(instance),
    _allocator(allocator)
{
}

Instance::~Instance()
{
    if (_instance)
    {
        //std::cout<<"Calling vkDestroyInstance"<<std::endl;
        vkDestroyInstance(_instance, _allocator);
    }
}

Instance::Result Instance::create(Names& instanceExtensions, Names& layers, AllocationCallbacks* allocator)
{
#if 0
    std::cout << "Instance::create()" << std::endl;
    std::cout << "instanceExtensions : " << std::endl;
    for (auto& name : instanceExtensions)
    {
        std::cout << "    " << name << std::endl;
    }

    std::cout << "layers : " << std::endl;
    for (auto& name : layers)
    {
        std::cout << "    " << name << std::endl;
    }
    std::cout << "allocator : " << allocator << std::endl;
#endif

    // application info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test";
    appInfo.pEngineName = "VulkanSceneGraph";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.empty() ? nullptr : instanceExtensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    createInfo.pNext = nullptr;

    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, allocator, &instance);
    if (result == VK_SUCCESS)
    {
        //std::cout << "Created VkInstance" << std::endl;
        return Result(new Instance(instance, allocator));
    }
    else
    {
        return Result("Error: vsg::Instance::create(...) failed to create VkInstance.", result);
    }
}
