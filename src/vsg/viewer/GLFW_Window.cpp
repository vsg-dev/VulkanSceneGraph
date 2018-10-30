/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include "GLFW_Window.h"

#include <vsg/core/observer_ptr.h>

#include <iostream>

using namespace glfw;

vsg::Names glfw::getInstanceExtensions()
{
    uint32_t glfw_count;
    const char** glfw_extensons = glfwGetRequiredInstanceExtensions(&glfw_count);
    return vsg::Names(glfw_extensons, glfw_extensons + glfw_count);
}

GLFW_Instance::GLFW_Instance()
{
    std::cout << "Calling glfwInit" << std::endl;
    glfwInit();
}

GLFW_Instance::~GLFW_Instance()
{
    std::cout << "Calling glfwTerminate()" << std::endl;
    glfwTerminate();
}

vsg::ref_ptr<glfw::GLFW_Instance> glfw::getGLFW_Instance()
{
    static vsg::observer_ptr<glfw::GLFW_Instance> s_glfw_Instance(new glfw::GLFW_Instance);
    return s_glfw_Instance;
}

namespace glfw
{
    class GLFWSurface : public vsg::Surface
    {
    public:
        GLFWSurface(vsg::Instance* instance, GLFWwindow* window, vsg::AllocationCallbacks* allocator = nullptr) :
            vsg::Surface(VK_NULL_HANDLE, instance, allocator)
        {
            if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS)
            {
                std::cout << "Failed to create window surface" << std::endl;
            }
            else
            {
                std::cout << "Created window surface" << std::endl;
            }
        }
    };
} // namespace glfw

GLFW_Window::GLFW_Window(GLFW_Instance* glfwInstance, GLFWwindow* window, vsg::Instance* instance, vsg::Surface* surface, vsg::PhysicalDevice* physicalDevice, vsg::Device* device, vsg::RenderPass* renderPass, bool debugLayersEnabled) :
    _glfwInstance(glfwInstance),
    _window(window)
{
    _instance = instance;
    _surface = surface;
    _physicalDevice = physicalDevice;
    _device = device;
    _renderPass = renderPass;
    _debugLayersEnabled = debugLayersEnabled;
}

GLFW_Window::Result GLFW_Window::create(uint32_t width, uint32_t height, bool debugLayer, bool apiDumpLayer, vsg::Window* shareWindow, vsg::AllocationCallbacks* allocator)
{
    std::cout << "Calling glfwCreateWindow(..)" << std::endl;

    vsg::ref_ptr<glfw::GLFW_Instance> glfwInstance = glfw::getGLFW_Instance();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* glfwWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    if (!glfwWindow) return Result("Error: vsg::GLFW_Window::create(...) GLFW failed to create Window.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    vsg::ref_ptr<GLFW_Window> window;

    if (shareWindow)
    {
        // use GLFW to create surface
        vsg::ref_ptr<vsg::Surface> surface(new glfw::GLFWSurface(shareWindow->instance(), glfwWindow, allocator));

        window = new GLFW_Window(glfwInstance, glfwWindow,
                                 shareWindow->instance(), shareWindow->surface(), shareWindow->physicalDevice(), shareWindow->device(), shareWindow->renderPass(), shareWindow->debugLayersEnabled());

        // share the _instance, _physicalDevice and _device;
        window->share(*shareWindow);

        // temporary hack to force vkGetPhysicalDeviceSurfaceSupportKHR to be called as the Vulkan
        // debug layer is complaining about vkGetPhysicalDeviceSurfaceSupportKHR not being called
        // for this _surface prior to swap chain creation
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(shareWindow->instance(), VK_QUEUE_GRAPHICS_BIT, surface);
    }
    else
    {
        vsg::Names instanceExtensions = glfw::getInstanceExtensions();

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
        if (!instance) return Result("Error: vsg::GLFW_Window::create(...) failed to create Window, unable to create Vulkan instance.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // use GLFW to create surface
        vsg::ref_ptr<vsg::Surface> surface(new glfw::GLFWSurface(instance, glfwWindow, allocator));
        if (!surface) return Result("Error: vsg::GLFW_Window::create(...) failed to create Window, unable to create GLFWSurface.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // set up device
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(instance, VK_QUEUE_GRAPHICS_BIT, surface);
        if (!physicalDevice) return Result("Error: vsg::GLFW_Window::create(...) failed to create Window, no Vulkan PhysicalDevice supported.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        vsg::ref_ptr<vsg::Device> device = vsg::Device::create(physicalDevice, validatedNames, deviceExtensions, allocator);
        if (!device) return Result("Error: vsg::GLFW_Window::create(...) failed to create Window, unable to create Vulkan logical Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // set up renderpass with the imageFormat that the swap chain will use
        vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*physicalDevice, *surface);
        VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
        VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT; //VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_SFLOAT_S8_UINT
        vsg::ref_ptr<vsg::RenderPass> renderPass = vsg::RenderPass::create(device, imageFormat.format, depthFormat, allocator);
        if (!renderPass) return Result("Error: vsg::GLFW_Window::create(...) failed to create Window, unable to create Vulkan RenderPass.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        window = new GLFW_Window(glfwInstance, glfwWindow,
                                 instance, surface, physicalDevice, device, renderPass, debugLayer);
    }

    window->buildSwapchain(width, height);

    return Result(window);
}

GLFW_Window::~GLFW_Window()
{
    clear();

    if (_window)
    {
        std::cout << "Calling glfwDestroyWindow(_window);" << std::endl;
        glfwDestroyWindow(_window);
    }
}

bool GLFW_Window::pollEvents()
{
    glfwPollEvents();
    return false;
}

bool GLFW_Window::resized() const
{
    int new_width, new_height;
    glfwGetWindowSize(_window, &new_width, &new_height);
    return (new_width != int(_extent2D.width) || new_height != int(_extent2D.height));
}

void GLFW_Window::resize()
{
    int new_width, new_height;
    glfwGetWindowSize(_window, &new_width, &new_height);
    buildSwapchain(new_width, new_height);
}
