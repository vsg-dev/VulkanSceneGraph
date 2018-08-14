#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/utils/CommandLine.h>

#include <vsg/nodes/Group.h>

#include <vsg/maths/transform.h>

#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/Image.h>
#include <vsg/vk/Sampler.h>
#include <vsg/vk/BufferView.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/BindIndexBuffer.h>

#include <vsg/viewer/Window.h>
#include <vsg/viewer/Viewer.h>

#include <osg2vsg/ImageUtils.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <set>
#include <chrono>
#include <cstring>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vsg
{
    ////////////////////////////////////////////////////////////////////
    //
    //  ostream implementation
    //
    void print(std::ostream& out, const VkPhysicalDeviceProperties& properties)
    {
        out<<"VkPhysicalDeviceProperties {"<<std::endl;
        out<<"   apiVersion = "<<properties.apiVersion<<std::endl;
        out<<"   driverVersion = "<<properties.driverVersion<<std::endl;
        out<<"   vendorID = "<<properties.vendorID<<std::endl;
        out<<"   deviceID = "<<properties.deviceID<<std::endl;
        out<<"   deviceType = "<<properties.deviceType<<std::endl;
        out<<"   deviceName = "<<properties.deviceName<<std::endl;
        out<<"   limits.maxDescriptorSetSamplers = "<<properties.limits.maxDescriptorSetSamplers<<std::endl;
        out<<"   limits.maxImageDimension1D = "<<properties.limits.maxImageDimension1D<<std::endl;
        out<<"   limits.maxImageDimension2D = "<<properties.limits.maxImageDimension2D<<std::endl;
        out<<"   limits.maxImageDimension3D = "<<properties.limits.maxImageDimension3D<<std::endl;
        out<<"   minUniformBufferOffsetAlignment = "<<properties.limits.minUniformBufferOffsetAlignment<<std::endl;
        out<<"}"<<std::endl;
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


};


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

class Window : public vsg::Window
{
public:

    Window() = delete;
    Window(const Window&) = delete;
    Window& operator = (const Window&) = delete;

    Window(uint32_t width, uint32_t height, bool debugLayer=false, bool apiDumpLayer=false, Window* shareWindow=nullptr) : _glwInstance(glfw::getGLFW_Instance())
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

            vsg::print(std::cout,"instanceExtensions",instanceExtensions);
            vsg::print(std::cout,"validatedNames",validatedNames);

            _instance = vsg::Instance::create(instanceExtensions, validatedNames);

            // use GLFW to create surface
            _surface = new glfw::GLFWSurface(_instance, _window, nullptr);


            // set up device
            _physicalDevice = vsg::PhysicalDevice::create(_instance, VK_QUEUE_GRAPHICS_BIT,  _surface);
            _device = vsg::Device::create(_physicalDevice, validatedNames, deviceExtensions);

            // set up renderpass with the imageFormat that the swap chain will use
            vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*_physicalDevice, *_surface);
            VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
            VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;//VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_SFLOAT_S8_UINT
            _renderPass = vsg::RenderPass::create(_device, imageFormat.format, depthFormat);
        }

        buildSwapchain(width, height);
    }

    virtual bool valid() const { return _window && !glfwWindowShouldClose(_window); }

    virtual bool resized() const
    {
        int new_width, new_height;
        glfwGetWindowSize(_window, &new_width, &new_height);
        return (new_width!=int(_extent2D.width) || new_height!=int(_extent2D.height));
    }

    virtual void resize()
    {
        int new_width, new_height;
        glfwGetWindowSize(_window, &new_width, &new_height);
        buildSwapchain(new_width, new_height);
    }

    operator GLFWwindow* () { return _window; }
    operator const GLFWwindow* () const { return _window; }

protected:
    virtual ~Window()
    {
        clear();

        if (_window)
        {
            std::cout<<"Calling glfwDestroyWindow(_window);"<<std::endl;
            glfwDestroyWindow(_window);
        }
    }

    vsg::ref_ptr<glfw::GLFW_Instance>   _glwInstance;
    GLFWwindow*                         _window;
};


}

namespace vsg
{
    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::tvec2<T>& vec)
    {
        output << vec.x << " " << vec.y;
        return output; // to enable cascading
    }

    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::tvec3<T>& vec)
    {
        output << vec.x << " " << vec.y<<" "<<vec.z;
        return output; // to enable cascading
    }

    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::tvec4<T>& vec)
    {
        output << vec.x << " " << vec.y<<" "<<vec.z<<" "<<vec.w;
        return output; // to enable cascading
    }

    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::tmat4<T>& mat)
    {
        output << std::endl;
        output << "    "<<mat(0,0)<< " " << mat(1,0)<<" "<<mat(2,0)<<" "<<mat(3,0)<<std::endl;
        output << "    "<<mat(0,1)<< " " << mat(1,1)<<" "<<mat(2,1)<<" "<<mat(3,1)<<std::endl;
        output << "    "<<mat(0,2)<< " " << mat(1,2)<<" "<<mat(2,2)<<" "<<mat(3,2)<<std::endl;
        output << "    "<<mat(0,3)<< " " << mat(1,3)<<" "<<mat(2,3)<<" "<<mat(3,3)<<std::endl;
        return output; // to enable cascading
    }

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
    bool apiDumpLayer = false;
    uint32_t width = 800;
    uint32_t height = 600;
    int numFrames=-1;
    bool useStagingBuffer = false;
    bool printFrameRate = false;
    int numWindows = 1;

    try
    {
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--debug","-d"))) debugLayer = true;
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--api","-a"))) { apiDumpLayer = true; debugLayer = true; }
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--window","-w"), width, height)) {}
        if (vsg::CommandLine::read(argc, argv, "-f", numFrames)) {}
        if (vsg::CommandLine::read(argc, argv, "-s")) { useStagingBuffer = true; }
        if (vsg::CommandLine::read(argc, argv, "--fr")) { printFrameRate = true; }
        if (vsg::CommandLine::read(argc, argv, "--num-windows", numWindows)) {}
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::Viewer> viewer = new vsg::Viewer;

    vsg::ref_ptr<glfw::Window> window = new glfw::Window(width, height, debugLayer, apiDumpLayer);
    viewer->addWindow(window);

    for(int i=1; i<numWindows; ++i)
    {
        viewer->addWindow(new glfw::Window(width, height, debugLayer, apiDumpLayer, window));
    }

    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = window->physicalDevice();
    vsg::ref_ptr<vsg::Device> device = window->device();
    vsg::ref_ptr<vsg::Surface> surface = window->surface();
    vsg::ref_ptr<vsg::RenderPass> renderPass = window->renderPass();

    vsg::ref_ptr<vsg::ShaderModule> vert = vsg::ShaderModule::read(device, VK_SHADER_STAGE_VERTEX_BIT, "main", "shaders/vert.spv");
    vsg::ref_ptr<vsg::ShaderModule> frag = vsg::ShaderModule::read(device, VK_SHADER_STAGE_FRAGMENT_BIT, "main", "shaders/frag.spv");
    if (!vert || !frag)
    {
        std::cout<<"Could not create shaders"<<std::endl;
        return 1;
    }
    vsg::ShaderModules shaderModules{vert, frag};
    vsg::ref_ptr<vsg::ShaderStages> shaderStages = new vsg::ShaderStages(shaderModules);

    VkQueue graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());
    VkQueue presentQueue = device->getQueue(physicalDevice->getPresentFamily());
    if (!graphicsQueue || !presentQueue)
    {
        std::cout<<"Required graphics/present queue not available!"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::CommandPool> commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());


    // set up vertex and index arrays
    vsg::ref_ptr<vsg::vec3Array> vertices = new vsg::vec3Array
    {
        {-0.5f, -0.5f, 0.0f},
        {0.5f,  -0.5f, 0.05f},
        {0.5f , 0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f},
        {-0.5f, -0.5f, -0.5f},
        {0.5f,  -0.5f, -0.5f},
        {0.5f , 0.5f, -0.5},
        {-0.5f, 0.5f, -0.5}
    };

    vsg::ref_ptr<vsg::vec3Array> colors = new vsg::vec3Array
    {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
    };

    vsg::ref_ptr<vsg::vec2Array> texcoords = new vsg::vec2Array
    {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    vsg::ref_ptr<vsg::ushortArray> indices = new vsg::ushortArray
    {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4
    };

    auto vertexBufferData = vsg::createBufferAndTransferData(device, commandPool, graphicsQueue, {vertices, colors, texcoords}, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    auto indexBufferData = vsg::createBufferAndTransferData(device, commandPool, graphicsQueue, {indices}, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);

    // set up uniforms
    vsg::ref_ptr<vsg::mat4Value> projMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> viewMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> modelMatrix = new vsg::mat4Value;

    auto uniformBufferData = vsg::createHostVisibleBuffer(device, {projMatrix, viewMatrix, modelMatrix}, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);

    //
    // set up texture image
    //
    vsg::ImageData imageData = osg2vsg::readImageFile(device, commandPool, graphicsQueue, "textures/lz.rgb");
    if (!imageData.valid())
    {
        std::cout<<"Texture not created"<<std::endl;
        return 1;
    }

    //
    // set up descriptor layout and descriptor set and pieline layout for uniforms
    //
    vsg::ref_ptr<vsg::DescriptorPool> descriptorPool = vsg::DescriptorPool::create(device, 1,
    {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
    });

    vsg::ref_ptr<vsg::DescriptorSetLayout> descriptorSetLayout = vsg::DescriptorSetLayout::create(device,
    {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}
    });

    vsg::ref_ptr<vsg::DescriptorSet> descriptorSet = vsg::DescriptorSet::create(device, descriptorPool, descriptorSetLayout,
    {
        new vsg::DescriptorBuffer(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBufferData),
        new vsg::DescriptorImage(3, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, {imageData})
    });

    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout = vsg::PipelineLayout::create(device, {descriptorSetLayout}, {});

    // setup binding of descriptors
    vsg::ref_ptr<vsg::CmdBindDescriptorSets> bindDescriptorSets = new vsg::CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, {descriptorSet}); // device dependent


    // set up graphics pipeline
    vsg::VertexInputState::Bindings vertexBindingsDescriptions
    {
        VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions
    {
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
        VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},
    };

    vsg::ref_ptr<vsg::GraphicsPipeline> pipeline = vsg::GraphicsPipeline::create(device, renderPass, pipelineLayout, // device dependent
    {
        shaderStages,  // device dependent
        new vsg::VertexInputState(vertexBindingsDescriptions, vertexAttributeDescriptions),// device independent
        new vsg::InputAssemblyState, // device independent
        new vsg::ViewportState(VkExtent2D{width, height}), // device independent
        new vsg::RasterizationState,// device independent
        new vsg::MultisampleState,// device independent
        new vsg::ColorBlendState,// device independent
        new vsg::DepthStencilState// device independent
    });

    // set up vertex buffer binding
    vsg::ref_ptr<vsg::BindVertexBuffers> bindVertexBuffers = new vsg::BindVertexBuffers(0, vertexBufferData);  // device dependent

    // set up index buffer binding
    vsg::ref_ptr<vsg::BindIndexBuffer> bindIndexBuffer = new vsg::BindIndexBuffer(indexBufferData.front(), VK_INDEX_TYPE_UINT16); // device dependent

    // set up drawing of the triangles
    vsg::ref_ptr<vsg::CmdDrawIndexed> drawIndexed = new vsg::CmdDrawIndexed(12, 1, 0, 0, 0); // device agnostic

    // set up what we want to render in a command graph
    // create command graph to contain all the Vulkan calls for specifically rendering the model
    vsg::ref_ptr<vsg::Group> commandGraph = new vsg::Group;

    // set up the state configuration
    commandGraph->addChild(pipeline);  // device dependent
    commandGraph->addChild(bindDescriptorSets);  // device dependent

    // add subgraph that represents the model to render
    vsg::ref_ptr<vsg::Group> model = new vsg::Group;
    commandGraph->addChild(model);

    // add the vertex and index buffer data
    model->addChild(bindVertexBuffers); // device dependent
    model->addChild(bindIndexBuffer); // device dependent

    // add the draw primitive command
    model->addChild(drawIndexed); // device independent

    //
    // end of initialize vulkan
    //
    /////////////////////////////////////////////////////////////////////

    auto startTime =std::chrono::steady_clock::now();
    float time = 0.0f;

    for(auto& window : viewer->windows())
    {
        window->populateCommandBuffers(commandGraph);
    }

    while (!viewer->done() && (numFrames<0 || (numFrames--)>0))
    {
        //std::cout<<"In main loop"<<std::endl;
        glfwPollEvents();

        float previousTime = time;
        time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now()-startTime).count();
        if (printFrameRate) std::cout<<"time = "<<time<<" fps="<<1.0/(time-previousTime)<<std::endl;

        // update
        (*projMatrix) = vsg::perspective(vsg::radians(45.0f), float(width)/float(height), 0.1f, 10.f);
        (*viewMatrix) = vsg::lookAt(vsg::vec3(2.0f, 2.0f, 2.0f), vsg::vec3(0.0f, 0.0f, 0.0f), vsg::vec3(0.0f, 0.0f, 1.0f));
        (*modelMatrix) = vsg::rotate(time * vsg::radians(90.0f), vsg::vec3(0.0f, 0.0, 1.0f));

        vsg::copyDataListToBuffers(uniformBufferData);

#if 0
        std::cout<<std::endl<<"New frame "<<time<<std::endl;
        std::cout<<"projMatrix = {"<<projMatrix->value()<<"}"<<std::endl;
        std::cout<<"viewMatrix = {"<<viewMatrix->value()<<"}"<<std::endl;
        std::cout<<"modelMatrix = {"<<modelMatrix->value()<<"}"<<std::endl;
#endif

        viewer->submitFrame(commandGraph);

    }


    // clean up done automatically thanks to ref_ptr<>
    return 0;
}
