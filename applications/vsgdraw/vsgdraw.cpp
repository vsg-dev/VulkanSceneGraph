#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/utils/CommandLine.h>

#include <vsg/vk/Instance.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/Pipeline.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Semaphore.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/////////////////////////////////////////////////////////////////////
//
// start of vulkan code
//

// interface
namespace vsg
{

}

// implementation
namespace vsg
{


}

//
// end of vulkan code
//
/////////////////////////////////////////////////////////////////////

namespace glfw
{

vsg::Names getInstanceExtensions()
{
    uint32_t glfw_count;
    const char** glfw_extensons = glfwGetRequiredInstanceExtensions(&glfw_count);
    return vsg::Names(glfw_extensons, glfw_extensons+glfw_count);
}

// forward declare
class GLFW_Instance;
static vsg::ref_ptr<glfw::GLFW_Instance> getGLFW_Instance();

class GLFW_Instance : public vsg::Object
{
public:
protected:
    GLFW_Instance()
    {
        std::cout<<"Calling glfwInit"<<std::endl;
        glfwInit();
    }

    virtual ~GLFW_Instance()
    {
        std::cout<<"Calling glfwTerminate()"<<std::endl;
        glfwTerminate();
    }

    friend vsg::ref_ptr<glfw::GLFW_Instance> getGLFW_Instance();
};

static vsg::ref_ptr<glfw::GLFW_Instance> getGLFW_Instance()
{
    static vsg::observer_ptr<glfw::GLFW_Instance> s_glfw_Instance = new glfw::GLFW_Instance;
    return s_glfw_Instance;
}

class Window : public vsg::Object
{
public:
    Window(uint32_t width, uint32_t height) : _instance(glfw::getGLFW_Instance())
    {
        std::cout<<"Calling glfwCreateWindow(..)"<<std::endl;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    }

    operator GLFWwindow* () { return _window; }
    operator const GLFWwindow* () const { return _window; }

protected:
    virtual ~Window()
    {
        if (_window)
        {
            std::cout<<"Calling glfwDestroyWindow(_window);"<<std::endl;
            glfwDestroyWindow(_window);
        }
    }

    vsg::ref_ptr<glfw::GLFW_Instance>   _instance;
    GLFWwindow*                         _window;
};


class GLFWSurface : public vsg::Surface
{
public:

    GLFWSurface(vsg::Instance* instance, GLFWwindow* window, vsg::AllocationCallbacks* allocator=nullptr) :
        vsg::Surface(instance, VK_NULL_HANDLE, allocator)
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

namespace vsg
{

    template< typename ... Args >
    std::string make_string(Args const& ... args )
    {
        std::ostringstream stream;
        using List= int[];
        (void) List {0, ( (void)(stream << args), 0 ) ... };

        return stream.str();
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

    vsg::ref_ptr<glfw::Window> window = new glfw::Window(width, height);


    /////////////////////////////////////////////////////////////////////
    //
    // start of initialize vulkan
    //

    vsg::Names instanceExtensions = glfw::getInstanceExtensions();

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

    vsg::ref_ptr<vsg::Instance> instance = vsg::Instance::create(instanceExtensions, validatedNames);

    // use GLFW to create surface
    vsg::ref_ptr<glfw::GLFWSurface> surface = new glfw::GLFWSurface(instance, *window, nullptr);

    // set up device
    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(instance, surface);
    vsg::ref_ptr<vsg::Device> device = vsg::Device::create(instance, physicalDevice, validatedNames, deviceExtensions);

    vsg::ref_ptr<vsg::ShaderModule> vert = vsg::ShaderModule::read(device, "shaders/vert.spv");
    vsg::ref_ptr<vsg::ShaderModule> frag = vsg::ShaderModule::read(device, "shaders/frag.spv");
    if (!vert || !frag)
    {
        std::cout<<"Could not create shaders"<<std::endl;
        return 1;
    }

    VkQueue graphicsQueue = vsg::createDeviceQueue(*device, physicalDevice->getGraphicsFamily());
    VkQueue presentQueue = vsg::createDeviceQueue(*device, physicalDevice->getPresentFamily());
    if (!graphicsQueue || !presentQueue)
    {
        std::cout<<"Required graphics/present queue not available!"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::Swapchain> swapchain = vsg::Swapchain::create(physicalDevice, device, surface, width, height);

    vsg::ref_ptr<vsg::RenderPass> renderPass = vsg::RenderPass::create(device, swapchain->getImageFormat());
    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout = new vsg::PipelineLayout(device);
    vsg::ref_ptr<vsg::Pipeline> pipeline = vsg::Pipeline::createGraphics(device, swapchain, renderPass, pipelineLayout, vert, frag);
    vsg::Framebuffers framebuffers = vsg::createFrameBuffers(device, swapchain, renderPass);
    vsg::ref_ptr<vsg::CommandPool> commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());

    vsg::ref_ptr<vsg::Semaphore> imageAvailableSemaphore = vsg::Semaphore::create(device);
    vsg::ref_ptr<vsg::Semaphore> renderFinishedSemaphore = vsg::Semaphore::create(device);


    //
    // end of initialize vulkan
    //
    /////////////////////////////////////////////////////////////////////


    // main loop
    int numFrames=10;
    while(!glfwWindowShouldClose(*window) && (numFrames--)>0)
    {
        std::cout<<"In main loop"<<std::endl;
        glfwPollEvents();

        // drawFrame
        uint32_t imageIndex;
        vkAcquireNextImageKHR(*device, *swapchain, std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {*imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

#if 0
        // TODO CommandBuffers
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
#endif

        VkSemaphore signalSemaphores[] = {*renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            std::cout<<"Error: failed to submit draw command buffer."<<std::endl;
            return 1;
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {*swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(presentQueue, &presentInfo);

        if (debugLayer)
        {
            vkQueueWaitIdle(presentQueue);
        }

    }

    vkDeviceWaitIdle(*device);

    // clean up done automatically thanks to ref_ptr<>
    return 0;
}