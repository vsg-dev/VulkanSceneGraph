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
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/CommandPool.h>

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

    template<class T, typename R, R validValue>
    class Result
    {
    public:

        Result(R result) : _printMessageOnError(true), _result(result) {}
        Result(const std::string& message, R result) : _printMessageOnError(true), _result(result), _message(message) {}
        Result(T* ptr) : _printMessageOnError(false), _result(validValue), _ptr(ptr) {}

        Result(const Result& rhs) :
            _printMessageOnError(rhs._printMessageOnError),
            _result(rhs._result),
            _message(rhs._message),
            _ptr(rhs._ptr)
        {
            rhs._printMessageOnError = false;
        }

        Result& operator = (const Result& rhs)
        {
            _printMessageOnError = rhs._printMessageOnError;
            _result = rhs._result;
            _message = rhs._message;
            _ptr = rhs._ptr;
            rhs._printMessageOnError = false;
            return *this;
        }

        ~Result()
        {
            if (_result!=validValue && _printMessageOnError)
            {
                if (_message.empty()) std::cerr<<"Warning: unhandled error value : "<<_result<<std::endl;
                else std::cerr<<_message<<std::endl;
            }
        }

        R result() { _printMessageOnError = false; return _result; }

        const std::string& message() { _printMessageOnError = false; return _message; }

        ref_ptr<T> object() { return _ptr; }

        operator ref_ptr<T> () { return _ptr; }

    protected:

        mutable bool    _printMessageOnError;
        R               _result;
        std::string     _message;
        ref_ptr<T>      _ptr;
    };


    using DeviceResult = Result<Device, VkResult, VK_SUCCESS>;

    DeviceResult createDevice(Instance* instance, PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator=nullptr)
    {
        if (instance) return new Device(instance, physicalDevice, layers, deviceExtensions, allocator);
        else return DeviceResult(make_string("Error: createDevice failed, return VkResult : ", VK_ERROR_DEVICE_LOST),VK_ERROR_DEVICE_LOST);
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


    std::cout<<std::endl;

    vsg::DeviceResult device2 = vsg::createDevice(instance.get(), physicalDevice.get(), validatedNames, deviceExtensions);

    std::cout<<"device2 result = "<<device2.result()<<std::endl;
    std::cout<<"device2 object = "<<device2.object().get()<<std::endl;

    {
    vsg::DeviceResult device3 = vsg::createDevice(nullptr, physicalDevice.get(), validatedNames, deviceExtensions);
    std::cout<<"device3 result = "<<device3.result()<<std::endl;
    std::cout<<"device3 object = "<<device3.object().get()<<std::endl;
    }

    vsg::ref_ptr<vsg::Device> device4 = vsg::createDevice(instance, physicalDevice, validatedNames, deviceExtensions);

    std::cout<<"device4 "<<device4.get()<<std::endl;

    vsg::ref_ptr<vsg::Device> device5 = vsg::createDevice(nullptr, physicalDevice.get(), validatedNames, deviceExtensions);

    std::cout<<"device5 "<<device5.get()<<std::endl;

    auto device6 = vsg::createDevice(nullptr, physicalDevice.get(), validatedNames, deviceExtensions);
    std::cout<<"device6 "<<device6.result()<<std::endl;

    {
    std::cout<<"device7 "<<std::endl;
    vsg::createDevice(nullptr, physicalDevice.get(), validatedNames, deviceExtensions);

    std::cout<<std::endl;
    }

    {
    std::cout<<"device8 "<<std::endl;
    vsg::createDevice(instance.get(), physicalDevice.get(), validatedNames, deviceExtensions);

    }
    std::cout<<std::endl;

    {
    std::cout<<"device9 "<<std::endl;
    vsg::DeviceResult temp = vsg::createDevice(nullptr, physicalDevice.get(), validatedNames, deviceExtensions);

    }
    std::cout<<std::endl;

    {
    std::cout<<"device10"<<std::endl;
    vsg::DeviceResult temp = vsg::createDevice(nullptr, physicalDevice.get(), validatedNames, deviceExtensions);
    std::cout<<"temp.result()"<<temp.result()<<std::endl;

    }
    std::cout<<std::endl;


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

    vsg::ref_ptr<vsg::RenderPass> renderPass = new vsg::RenderPass(device.get(), swapchain->getImageFormat());
    std::cout<<"Created RenderPass"<<std::endl;

    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout = new vsg::PipelineLayout(device.get());

    std::cout<<"Created PipelineLayout "<<pipelineLayout.get()<<std::endl;

    vsg::ref_ptr<vsg::Pipeline> pipeline = vsg::createGraphicsPipeline(device.get(), swapchain.get(), renderPass.get(), pipelineLayout.get(), vert.get(), frag.get());

    std::cout<<"Created GraphicsPipline "<<pipeline.get()<<std::endl;

    vsg::Framebuffers framebuffers = vsg::createFrameBuffers(device.get(), swapchain.get(), renderPass.get());
    std::cout<<"Created Framebuffers "<<framebuffers.size()<<std::endl;

    vsg::ref_ptr<vsg::CommandPool> commandPool = new vsg::CommandPool(device.get(), physicalDevice-> getGraphicsFamily());

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