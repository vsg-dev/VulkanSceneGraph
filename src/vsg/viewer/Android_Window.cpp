/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include "Android_Window.h"

#include <android/log.h>

#include <vsg/vk/Extensions.h>
#include <vsg/core/observer_ptr.h>

#include <iostream>

using namespace vsg;
using namespace vsgAndroid;

#define LOG(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

namespace vsgAndroid
{
    vsg::Names vsgAndroid::getInstanceExtensions()
    {
        // check the extensions are avaliable first
        Names requiredExtensions = { "VK_KHR_surface", "VK_KHR_android_surface" };

        if (!vsg::isExtensionListSupported(requiredExtensions))
        {
            std::cout << "Error: vsg::getInstanceExtensions(...) unable to create window, VK_KHR_surface or VK_KHR_android_surface not supported." << std::endl;
            return Names();
        }

        return requiredExtensions;
    }

    class AndroidSurface : public vsg::Surface
    {
    public:
        AndroidSurface(vsg::Instance* instance, ANativeWindow* window, vsg::AllocationCallbacks* allocator = nullptr) :
            vsg::Surface(VK_NULL_HANDLE, instance, allocator)
        {
            VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext = nullptr;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.window = window;

            auto result = vkCreateAndroidSurfaceKHR(*instance, &surfaceCreateInfo, nullptr, &_surface);
        }
    };

} // namespace vsg

Android_Window::Android_Window(ANativeWindow* window, vsg::Instance* instance, vsg::Surface* surface, vsg::PhysicalDevice* physicalDevice, vsg::Device* device, vsg::RenderPass* renderPass, bool debugLayersEnabled) :
    _window(window)
{
    _instance = instance;
    _surface = surface;
    _physicalDevice = physicalDevice;
    _device = device;
    _renderPass = renderPass;
    _debugLayersEnabled = debugLayersEnabled;
}

Android_Window::Result Android_Window::create(const Window::Traits& traits, bool debugLayer, bool apiDumpLayer, vsg::AllocationCallbacks* allocator)
{
    std::cout << "Calling CreateWindowEx(..)" << std::endl;

    /*if(!traits.nativeHandle.has_value())
    {
        return Result("Error: vsg::Android_Window::create(...) failed to create Window, Android requires a NativeWindow passed via traits.nativeHandle.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }*/
    
    //ANativeWindow* nativeWindow = *std::any_cast<ANativeWindow*>(&traits.nativeHandle);
    ANativeWindow* nativeWindow = static_cast<ANativeWindow*>(traits.nativeWindow);
    
    if(nativeWindow == nullptr)
    {
        return Result("Error: vsg::Android_Window::create(...) failed to create Window, traits.nativeHandle is not a valid ANativeWindow.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    // we could get the width height from the window?
    uint32_t finalWidth = traits.width;
    uint32_t finalHeight = traits.height;

    vsg::ref_ptr<Android_Window> window;

    if (traits.shareWindow)
    {
        // use GLFW to create surface
        vsg::ref_ptr<vsg::Surface> surface(new vsgAndroid::AndroidSurface(traits.shareWindow->instance(), nativeWindow, allocator));

        window = new Android_Window(nativeWindow, traits.shareWindow->instance(), traits.shareWindow->surface(), traits.shareWindow->physicalDevice(), traits.shareWindow->device(), traits.shareWindow->renderPass(), traits.shareWindow->debugLayersEnabled());

        // share the _instance, _physicalDevice and _device;
        window->share(*traits.shareWindow);

        // temporary hack to force vkGetPhysicalDeviceSurfaceSupportKHR to be called as the Vulkan
        // debug layer is complaining about vkGetPhysicalDeviceSurfaceSupportKHR not being called
        // for this _surface prior to swap chain creation
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(traits.shareWindow->instance(), VK_QUEUE_GRAPHICS_BIT, surface);
    }
    else
    {
        vsg::Names instanceExtensions =  vsgAndroid::getInstanceExtensions();

        vsg::Names requestedLayers;
        if (debugLayer)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
            if (apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
        }

        vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);

        vsg::Names deviceExtensions;
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vsg::ref_ptr<vsg::Instance> instance = vsg::Instance::create(instanceExtensions, validatedNames, allocator);
        if (!instance) return Result("Error: vsg::Android_Window::create(...) failed to create Window, unable to create Vulkan instance.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // use GLFW to create surface
        vsg::ref_ptr<vsg::Surface> surface(new vsgAndroid::AndroidSurface(instance, nativeWindow, allocator));
        if (!surface) return Result("Error: vsg::Android_Window::create(...) failed to create Window, unable to create Win32Surface.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // set up device
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(instance, VK_QUEUE_GRAPHICS_BIT, surface);
        if (!physicalDevice) return Result("Error: vsg::Android_Window::create(...) failed to create Window, no Vulkan PhysicalDevice supported.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        vsg::ref_ptr<vsg::Device> device = vsg::Device::create(physicalDevice, validatedNames, deviceExtensions, allocator);
        if (!device) return Result("Error: vsg::Android_Window::create(...) failed to create Window, unable to create Vulkan logical Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // set up renderpass with the imageFormat that the swap chain will use
        vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*physicalDevice, *surface);
        VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
        VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT; //VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_SFLOAT_S8_UINT
        vsg::ref_ptr<vsg::RenderPass> renderPass = vsg::RenderPass::create(device, imageFormat.format, depthFormat, allocator);
        if (!renderPass) return Result("Error: vsg::Android_Window::create(...) failed to create Window, unable to create Vulkan RenderPass.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        window = new Android_Window(nativeWindow, instance, surface, physicalDevice, device, renderPass, debugLayer);
    }

    window->buildSwapchain(finalWidth, finalHeight);

    return Result(window);
}

Android_Window::~Android_Window()
{
    clear();

    if (_window != nullptr)
    {
        std::cout << "Calling DestroyWindow(_window);" << std::endl;
    }
}

bool Android_Window::pollEvents()
{
    return false;
}

bool Android_Window::resized() const
{
    // just hack resize for now
    int width = int(_extent2D.width);
    int height = int(_extent2D.height);

    return (width != int(_extent2D.width) || height != int(_extent2D.height));
}

void Android_Window::resize()
{
    int width = int(_extent2D.width);
    int height = int(_extent2D.height);

    buildSwapchain(width, height);
}
