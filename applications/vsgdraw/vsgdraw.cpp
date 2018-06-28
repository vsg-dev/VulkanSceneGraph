#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/utils/CommandLine.h>

#include <vsg/vk/Instance.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/Pipeline.h>

#include <iostream>
#include <fstream>
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

    // is this even neccessary?
    class Viewport : public vsg::Object, public VkViewport
    {
    public:
        Viewport() : VkViewport{} {}

    protected:
        virtual ~Viewport() {}
    };

    class Framebuffer : public Object
    {
    public:
        Framebuffer(Device* device, VkFramebuffer framebuffer, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkFramebuffer () const { return _framebuffer; }

    protected:
        virtual ~Framebuffer();

        ref_ptr<Device>         _device;
        VkFramebuffer           _framebuffer;
        VkAllocationCallbacks*  _pAllocator;
    };

}

// implementation
namespace vsg
{

Framebuffer::Framebuffer(Device* device, VkFramebuffer framebuffer, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _framebuffer(framebuffer),
    _pAllocator(pAllocator)
{
}

Framebuffer::~Framebuffer()
{
    if (_framebuffer)
    {
        std::cout<<"Calling vkDestroyFramebuffer"<<std::endl;
        vkDestroyFramebuffer(*_device, _framebuffer, _pAllocator);
    }
}

using Framebuffers = std::vector<ref_ptr<Framebuffer>>;

Framebuffers createFrameBuffers(Device* device, Swapchain* swapchain, RenderPass* renderPass, VkAllocationCallbacks* pAllocator=nullptr)
{
    const Swapchain::ImageViews& imageViews = swapchain->getImageViews();
    const VkExtent2D& extent = swapchain->getExtent();

    Framebuffers framebuffers;
    for(auto imageView : imageViews)
    {
        VkImageView attachments[] =
        {
            *imageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        VkFramebuffer framebuffer;
        if (vkCreateFramebuffer(*device,&framebufferInfo, pAllocator, &framebuffer) == VK_SUCCESS)
        {
            framebuffers.push_back(new Framebuffer(device, framebuffer, pAllocator));
        }
        else
        {
            std::cout<<"Failing to create framebuffer for "<<&imageView<<std::endl;
        }
    }
    return framebuffers;
}

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

    GLFWSurface(vsg::Instance* instance, GLFWwindow* window, VkAllocationCallbacks* pAllocator=nullptr) :
        vsg::Surface(instance, VK_NULL_HANDLE, pAllocator)
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

    vsg::ref_ptr<vsg::Instance> instance = new vsg::Instance(instanceExtensions, validatedNames);

    // use GLFW to create surface
    vsg::ref_ptr<glfw::GLFWSurface> surface = new glfw::GLFWSurface(instance.get(), *window, nullptr);

    // set up device
    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = new vsg::PhysicalDevice(instance.get(), surface.get());
    if (!physicalDevice->complete())
    {
        std::cout<<"No VkPhysicalDevice available!"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::Device> device = new vsg::Device(instance.get(), physicalDevice.get(), validatedNames, deviceExtensions);

    VkQueue graphicsQueue = vsg::createDeviceQueue(*device, physicalDevice->getGraphicsFamily());
    if (!graphicsQueue)
    {
        std::cout<<"No Graphics queue available!"<<std::endl;
        return 1;
    }

    VkQueue presentQueue = vsg::createDeviceQueue(*device, physicalDevice->getPresentFamily());
    if (!presentQueue)
    {
        std::cout<<"No Present queue available!"<<std::endl;
        return 1;
    }

    std::cout<<"Created graphicsQueue="<<graphicsQueue<<", presentQueue="<<presentQueue<<std::endl;

    vsg::ref_ptr<vsg::Swapchain> swapchain = new vsg::Swapchain(physicalDevice.get(), device.get(), surface.get(), width, height);
    std::cout<<"Created swapchain"<<std::endl;

    vsg::ref_ptr<vsg::ShaderModule> vert = vsg::readShaderModule(device.get(), "shaders/vert.spv");
    if (!vert)
    {
        std::cout<<"Could not find vertex shader"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::ShaderModule> frag = vsg::readShaderModule(device.get(), "shaders/frag.spv");
    if (!frag)
    {
        std::cout<<"Could not find fragment shader"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::Viewport> viewport = new vsg::Viewport;
    (*viewport).width = 10.0f;

    vsg::ref_ptr<vsg::RenderPass> renderPass = new vsg::RenderPass(device.get(), swapchain->getImageFormat());
    std::cout<<"Created RenderPass"<<std::endl;

    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout = new vsg::PipelineLayout(device.get());

    std::cout<<"Created PipelineLayout "<<pipelineLayout.get()<<std::endl;

    vsg::ref_ptr<vsg::Pipeline> pipeline = vsg::createGraphicsPipeline(device.get(), swapchain.get(), renderPass.get(), pipelineLayout.get(), vert.get(), frag.get());

    std::cout<<"Created GraphicsPipline "<<pipeline.get()<<std::endl;

    vsg::Framebuffers framebuffers = vsg::createFrameBuffers(device.get(), swapchain.get(), renderPass.get());
    std::cout<<"Created Framebuffers "<<framebuffers.size()<<std::endl;


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
    }

    // clean up done automatically thanks to ref_ptr<>
    return 0;
}