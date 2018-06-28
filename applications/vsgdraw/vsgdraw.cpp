#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/utils/CommandLine.h>

#include <vsg/vk/Instance.h>
#include <vsg/vk/Surface.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/ShaderModule.h>

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

    class PipelineLayout : public vsg::Object
    {
    public:
        PipelineLayout(Device* device, VkPipelineLayout pipelineLayout, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkPipelineLayout () const { return _pipelineLayout; }

    protected:
        virtual ~PipelineLayout();

        ref_ptr<Device>         _device;
        VkPipelineLayout        _pipelineLayout;
        VkAllocationCallbacks*  _pAllocator;
    };

    class RenderPass : public vsg::Object
    {
    public:
        RenderPass(Device* device, VkRenderPass renderPass, VkAllocationCallbacks* pAllocator=nullptr);

        RenderPass(Device* device, VkFormat imageFormat, VkAllocationCallbacks* pAllocator=nullptr);


        operator VkRenderPass () const { return _renderPass; }

    protected:
        virtual ~RenderPass();

        ref_ptr<Device>         _device;
        VkRenderPass            _renderPass;
        VkAllocationCallbacks*  _pAllocator;
    };

}

// implementation
namespace vsg
{

PipelineLayout::PipelineLayout(Device* device, VkPipelineLayout pipelineLayout, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _pipelineLayout(pipelineLayout),
    _pAllocator(pAllocator)
{
}

PipelineLayout::~PipelineLayout()
{
    if (_pipelineLayout)
    {
        std::cout<<"Calling vkDestroyPipelineLayout"<<std::endl;
        vkDestroyPipelineLayout(*_device, _pipelineLayout, _pAllocator);
    }
}


RenderPass::RenderPass(Device* device, VkRenderPass renderPass, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _renderPass(renderPass),
    _pAllocator(pAllocator)
{
}

RenderPass::RenderPass(Device* device, VkFormat imageFormat, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _renderPass(VK_NULL_HANDLE),
    _pAllocator(pAllocator)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(*device, &renderPassInfo, pAllocator, &_renderPass) != VK_SUCCESS)
    {
        std::cout<<"Failed to create VkRenderPass."<<std::endl;
    }
}

RenderPass::~RenderPass()
{
    if (_renderPass)
    {
        std::cout<<"Calling vkDestroyRenderPass"<<std::endl;
        vkDestroyRenderPass(*_device, _renderPass, _pAllocator);
    }
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
    vsg::ref_ptr<vsg::ShaderModule> frag = vsg::readShaderModule(device.get(), "shaders/frag.spv");


    vsg::ref_ptr<vsg::Viewport> viewport = new vsg::Viewport;
    (*viewport).width = 10.0f;

    vsg::ref_ptr<vsg::RenderPass> renderPass = new vsg::RenderPass(device.get(), swapchain->getImageFormat());
    std::cout<<"Created RenderPass"<<std::endl;


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