
#include "GLFW_Window.h"

#include <vsg/core/observer_ptr.h>

#include <iostream>

namespace glfw
{

vsg::Names getInstanceExtensions()
{
    uint32_t glfw_count;
    const char** glfw_extensons = glfwGetRequiredInstanceExtensions(&glfw_count);
    return vsg::Names(glfw_extensons, glfw_extensons+glfw_count);
}

GLFW_Instance::GLFW_Instance()
{
    std::cout<<"Calling glfwInit"<<std::endl;
    glfwInit();
}

GLFW_Instance::~GLFW_Instance()
{
    std::cout<<"Calling glfwTerminate()"<<std::endl;
    glfwTerminate();
}

vsg::ref_ptr<glfw::GLFW_Instance> getGLFW_Instance()
{
    static vsg::observer_ptr<glfw::GLFW_Instance> s_glfw_Instance = new glfw::GLFW_Instance;
    return s_glfw_Instance;
}


class GLFWSurface : public vsg::Surface
{
public:

    GLFWSurface(vsg::Instance* instance, GLFWwindow* window, vsg::AllocationCallbacks* allocator=nullptr) :
        vsg::Surface(VK_NULL_HANDLE, instance, allocator)
    {
        if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS)
        {
            std::cout<<"Failed to create window surface"<<std::endl;
        }
        else
        {
            std::cout<<"Created window surface"<<std::endl;
        }
    }
};

GLFW_Window::GLFW_Window(uint32_t width, uint32_t height, bool debugLayer, bool apiDumpLayer, vsg::Window* shareWindow, vsg::AllocationCallbacks* allocator) :
    _glwInstance(glfw::getGLFW_Instance())
{
    std::cout<<"Calling glfwCreateWindow(..)"<<std::endl;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    _debugLayersEnabled = debugLayer;

    if (shareWindow)
    {
        // share the _instance, _physicalDevice and _devoce;
        share(*shareWindow);

        // use GLFW to create surface
        _surface = new glfw::GLFWSurface(_instance, _window, nullptr);

        // temporary hack to force vkGetPhysicalDeviceSurfaceSupportKHR to be called as the Vulkan
        // debug layer is complaining about vkGetPhysicalDeviceSurfaceSupportKHR not being called
        // for this _surface prior to swap chain creation
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(_instance, VK_QUEUE_GRAPHICS_BIT, _surface);
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

        _instance = vsg::Instance::create(instanceExtensions, validatedNames, allocator);

        // use GLFW to create surface
        _surface = new glfw::GLFWSurface(_instance, _window, allocator);


        // set up device
        _physicalDevice = vsg::PhysicalDevice::create(_instance, VK_QUEUE_GRAPHICS_BIT,  _surface);
        _device = vsg::Device::create(_physicalDevice, validatedNames, deviceExtensions, allocator);

        // set up renderpass with the imageFormat that the swap chain will use
        vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*_physicalDevice, *_surface);
        VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
        VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;//VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_SFLOAT_S8_UINT
        _renderPass = vsg::RenderPass::create(_device, imageFormat.format, depthFormat, allocator);
    }

    buildSwapchain(width, height);
}

GLFW_Window::~GLFW_Window()
{
    clear();

    if (_window)
    {
        std::cout<<"Calling glfwDestroyWindow(_window);"<<std::endl;
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
    return (new_width!=int(_extent2D.width) || new_height!=int(_extent2D.height));
}

void GLFW_Window::resize()
{
    int new_width, new_height;
    glfwGetWindowSize(_window, &new_width, &new_height);
    buildSwapchain(new_width, new_height);
}



}

