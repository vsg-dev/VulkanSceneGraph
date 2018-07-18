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
#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/CommandBuffers.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

    using DispatchList = std::vector<ref_ptr<Dispatch>>;

    template<typename T, VkStructureType type>
    class Info : public Object, public T
    {
    public:
        Info() : T{type} {}

    protected:
        virtual ~Info() {}

    };

    using RenderPassBeginInfo = Info<VkRenderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO>;

    VkResult populateCommandBuffer(VkCommandBuffer commandBuffer, RenderPass* renderPass, Framebuffer* framebuffer, Swapchain* swapchain, const DispatchList& dispatchList)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        VkResult result;;
        if ((result = vkBeginCommandBuffer(commandBuffer, &beginInfo)) != VK_SUCCESS)
        {
            std::cout<<"Error: could not begin command buffer."<<std::endl;
            return result;
        }

        ref_ptr<RenderPassBeginInfo> renderPassInfo = new RenderPassBeginInfo;
        renderPassInfo->renderPass = *renderPass;
        renderPassInfo->framebuffer = *framebuffer;
        renderPassInfo->renderArea.offset = {0, 0};
        renderPassInfo->renderArea.extent = swapchain->getExtent();

        VkClearValue clearColor = {0.2f, 0.2f, 0.4f, 1.0f};
        renderPassInfo->clearValueCount = 1;
        renderPassInfo->pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffer, renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        for(auto dispatch : dispatchList)
        {
            dispatch->dispatch(commandBuffer);
        }

        vkCmdEndRenderPass(commandBuffer);

        if ((result = vkEndCommandBuffer(commandBuffer)) != VK_SUCCESS)
        {
            std::cout<<"Error: could not end command buffer."<<std::endl;
            return result;
        }
    }

    class VulkanWindowObjects : public Object
    {
    public:

        VulkanWindowObjects(PhysicalDevice* physicalDevice, Device* device, Surface* surface, CommandPool* commandPool, RenderPass* renderPass, const DispatchList& dispatchList, uint32_t width, uint32_t height)
        {
            // keep device and commandPool around to enable vkFreeCommandBuffers call in destructor
            _device = device;
            _commandPool = commandPool;
            _renderPass = renderPass;

            // create all the window related Vulkan objects
            swapchain = Swapchain::create(physicalDevice, device, surface, width, height);
            framebuffers = createFrameBuffers(device, swapchain, renderPass);

            std::cout<<"swapchain->getImageFormat()="<<swapchain->getImageFormat()<<std::endl;

            commandBuffers = CommandBuffers::create(device, commandPool, framebuffers.size());
            if (commandBuffers)
            {
                for(size_t i=0; i<commandBuffers->size(); ++i)
                {
                    populateCommandBuffer((*commandBuffers)[i], renderPass, framebuffers[i], swapchain, dispatchList);
                }
            }
        }

        ref_ptr<Device>             _device;
        ref_ptr<CommandPool>        _commandPool;
        ref_ptr<RenderPass>         _renderPass;

        ref_ptr<Swapchain>          swapchain;
        Framebuffers                framebuffers;
        ref_ptr<CommandBuffers>     commandBuffers;
    };
}



int main(int argc, char** argv)
{
    bool debugLayer = false;
    uint32_t width = 800;
    uint32_t height = 600;
    int numFrames=-1;

    try
    {
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--debug","-d"))) debugLayer = true;
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--window","-w"), width, height)) {}
        if (vsg::CommandLine::read(argc, argv, "-f", numFrames)) {}
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


    vsg::ref_ptr<vsg::ShaderModule> vert = vsg::ShaderModule::read(device, VK_SHADER_STAGE_VERTEX_BIT, "main", "shaders/vert.spv");
    vsg::ref_ptr<vsg::ShaderModule> frag = vsg::ShaderModule::read(device, VK_SHADER_STAGE_FRAGMENT_BIT, "main", "shaders/frag.spv");
    if (!vert || !frag)
    {
        std::cout<<"Could not create shaders"<<std::endl;
        return 1;
    }
    vsg::ShaderModules shaderModules{vert, frag};
    vsg::ref_ptr<vsg::ShaderStages> shaderStages = new vsg::ShaderStages(shaderModules);

    VkQueue graphicsQueue = vsg::createDeviceQueue(*device, physicalDevice->getGraphicsFamily());
    VkQueue presentQueue = vsg::createDeviceQueue(*device, physicalDevice->getPresentFamily());
    if (!graphicsQueue || !presentQueue)
    {
        std::cout<<"Required graphics/present queue not available!"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::CommandPool> commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());
    vsg::ref_ptr<vsg::Semaphore> imageAvailableSemaphore = vsg::Semaphore::create(device);
    vsg::ref_ptr<vsg::Semaphore> renderFinishedSemaphore = vsg::Semaphore::create(device);


    // set up vertex arrays
    vsg::ref_ptr<vsg::vec2Array> vertices = new vsg::vec2Array(4);
    vsg::ref_ptr<vsg::vec3Array> colors = new vsg::vec3Array(4);

    vertices->set(0, {-0.5f, -0.5f});
    vertices->set(1, {0.5f,  -0.5f});
    vertices->set(2, {0.5f , 0.5f});
    vertices->set(3, {-0.5f, 0.5f});

    colors->set(0, {1.0f, 0.0f, 0.0f});
    colors->set(1, {0.0f, 1.0f, 0.0f});
    colors->set(2, {0.0f, 0.0f, 1.0f});
    colors->set(3, {1.0f, 1.0f, 1.0f});

    vsg::ref_ptr<vsg::ushortArray> indices = new vsg::ushortArray(6);
    indices->set(0, 0);
    indices->set(1, 1);
    indices->set(2, 2);
    indices->set(3, 2);
    indices->set(4, 3);
    indices->set(5, 0);

#if 1
    // copy the vertex data to local vertex buffer
    vsg::ref_ptr<vsg::Buffer> vertexBuffer = vsg::Buffer::create(device, vertices->dataSize()+colors->dataSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::ref_ptr<vsg::DeviceMemory> vertexBufferMemory =  vsg::DeviceMemory::create(physicalDevice, device, vertexBuffer,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(*device, *vertexBuffer, *vertexBufferMemory, 0);
    vertexBufferMemory->copy(0, vertices->dataSize(), vertices->dataPointer());
    vertexBufferMemory->copy(vertices->dataSize(), colors->dataSize(), colors->dataPointer());

    vsg::ref_ptr<vsg::Buffer> indexBuffer = vsg::Buffer::create(device, indices->dataSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::ref_ptr<vsg::DeviceMemory> indexBufferMemory =  vsg::DeviceMemory::create(physicalDevice, device, indexBuffer,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(*device, *indexBuffer, *indexBufferMemory, 0);
    indexBufferMemory->copy(0, indices->dataSize(), indices->dataPointer());

    using DataList = std::vector<vsg::ref_ptr<vsg::Data>>;
    DataList uniforms;

    vsg::ref_ptr<vsg::mat4Value> modelMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> viewMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> projMatrix = new vsg::mat4Value;

    uniforms.push_back(modelMatrix);
    uniforms.push_back(viewMatrix);
    uniforms.push_back(projMatrix);

    std::size_t uniformTotalSize = std::accumulate(uniforms.begin(), uniforms.end(), 0, [](std::size_t lhs, vsg::Data* rhs) { return lhs+rhs->dataSize(); });
    std::cout<<"uniformTotalSize = "<<uniformTotalSize<<std::endl;

    vsg::ref_ptr<vsg::Buffer> uniformBuffer = vsg::Buffer::create(device, uniformTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::ref_ptr<vsg::DeviceMemory> uniformBufferMemory =  vsg::DeviceMemory::create(physicalDevice, device, uniformBuffer,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(*device, *uniformBuffer, *indexBufferMemory, 0);

    std::size_t offset = 0;
    for(vsg::Data* data : uniforms)
    {
        std::size_t size = data->dataSize();
        uniformBufferMemory->copy(offset, size, data->dataPointer());
        offset += size;
    }

    #else
    // copy the vertex data to a stageing buffer, then submit a command to copy it to the final vertex buffer hosted in GPU local memory.
    VkDeviceSize vertices_offset = 0;
    VkDeviceSize colors_offset = vertices->dataSize();
    VkDeviceSize totalSize = colors_offset + colors->dataSize();

    vsg::ref_ptr<vsg::Buffer> stagingBuffer = vsg::Buffer::create(device, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);

    vsg::ref_ptr<vsg::DeviceMemory> stagingBufferMemory = vsg::DeviceMemory::create(physicalDevice, device, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(*device, *stagingBuffer, *stagingBufferMemory, 0);
    stagingBufferMemory->copy(0, vertices->dataSize(), vertices->data());
    stagingBufferMemory->copy(vertices->dataSize(), colors->dataSize(), colors->data());

    vsg::ref_ptr<vsg::Buffer> vertexBuffer = vsg::Buffer::create(device, totalSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::ref_ptr<vsg::DeviceMemory> vertexBufferMemory =  vsg::DeviceMemory::create(physicalDevice, device, vertexBuffer,  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vsg::ref_ptr<vsg::CommandBuffers> transferCommand = vsg::CommandBuffers::create(device, commandPool, 1);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(transferCommand->at(0), &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = totalSize;
        vkBindBufferMemory(*device, *vertexBuffer, *vertexBufferMemory, 0);
        vkCmdCopyBuffer(transferCommand->at(0), *stagingBuffer, *vertexBuffer, 1, &copyRegion);

    vkEndCommandBuffer(transferCommand->at(0));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = transferCommand->size();
    submitInfo.pCommandBuffers = transferCommand->data();

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
#endif

    vsg::ref_ptr<vsg::CmdBindVertexBuffers> bindVertexBuffers = new vsg::CmdBindVertexBuffers;
    bindVertexBuffers->add(vertexBuffer, 0);
    bindVertexBuffers->add(vertexBuffer, vertices->dataSize());

    vsg::ref_ptr<vsg::CmdBindIndexBuffer> bindIndexBuffer = new vsg::CmdBindIndexBuffer(indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vsg::VertexInputState::Bindings vertexBindingsDescriptions{VkVertexInputBindingDescription{0, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}, VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}};
    vsg::VertexInputState::Attributes vertexAttrobiteDescriptions{VkVertexInputAttributeDescription{0, 0,VK_FORMAT_R32G32_SFLOAT, 0}, VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}};

    vsg::ref_ptr<vsg::CmdDrawIndexed> drawIndexed = new vsg::CmdDrawIndexed(6, 1, 0, 0, 0);

    // setup pipeline
    vsg::GraphicsPipelineStates pipelineStates;
    pipelineStates.push_back(shaderStages);
    pipelineStates.push_back(new vsg::VertexInputState(vertexBindingsDescriptions, vertexAttrobiteDescriptions));
    pipelineStates.push_back(new vsg::InputAssemblyState);
    pipelineStates.push_back(new vsg::ViewportState(VkExtent2D{width, height}));
    pipelineStates.push_back(new vsg::RasterizationState);
    pipelineStates.push_back(new vsg::MultisampleState);
    pipelineStates.push_back(new vsg::ColorBlendState);

    vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*physicalDevice, *surface);
    VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
    vsg::ref_ptr<vsg::RenderPass> renderPass = vsg::RenderPass::create(device, imageFormat.format);

    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout = new vsg::PipelineLayout(device);
    vsg::ref_ptr<vsg::Pipeline> pipeline = vsg::createGraphicsPipeline(device, renderPass, pipelineLayout, pipelineStates);


    // set up what we want to render
    vsg::DispatchList dispatchList;
    dispatchList.push_back(pipeline);
    dispatchList.push_back(bindVertexBuffers);
    dispatchList.push_back(bindIndexBuffer);
    dispatchList.push_back(drawIndexed);

    vsg::ref_ptr<vsg::VulkanWindowObjects> vwo = new vsg::VulkanWindowObjects(physicalDevice, device, surface, commandPool, renderPass, dispatchList, width, height);

    //
    // end of initialize vulkan
    //
    /////////////////////////////////////////////////////////////////////


    // main loop
    while(!glfwWindowShouldClose(*window) && (numFrames<0 || (numFrames--)>0))
    {
        //std::cout<<"In main loop"<<std::endl;
        glfwPollEvents();

        bool needToRegerateVulkanWindowObjects = false;

        int new_width, new_height;
        glfwGetWindowSize(*window, &new_width, &new_height);
        if (new_width!=int(width) || new_height!=int(height))
        {
            std::cout<<"Warning: window resized to "<<new_width<<", "<<new_height<<std::endl;
            needToRegerateVulkanWindowObjects = true;
        }

        uint32_t imageIndex;
        if (!needToRegerateVulkanWindowObjects)
        {
            // drawFrame
            VkResult result = vkAcquireNextImageKHR(*device, *(vwo->swapchain), std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                needToRegerateVulkanWindowObjects = true;
                std::cout<<"Warning: Image out of data, need to recreate swap chain and assoicated dependencies."<<std::endl;
            }
            else if (result != VK_SUCCESS)
            {
                needToRegerateVulkanWindowObjects = true;
                std::cout<<"Warning: failed to aquire swap chain image."<<std::endl;
            }
        }

        if (needToRegerateVulkanWindowObjects)
        {
            vkDeviceWaitIdle(*device);

            // clean up previous VulkanWindowObjects
            vwo = nullptr;

            // create new VulkanWindowObjects
            width = new_width;
            height = new_height;
            vwo = new vsg::VulkanWindowObjects(physicalDevice, device, surface, commandPool, renderPass, dispatchList, width, height);

            VkResult result = vkAcquireNextImageKHR(*device, *(vwo->swapchain), std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
            if (result != VK_SUCCESS)
            {
                std::cout<<"Warning: could not recreate swap chain image."<<std::endl;
                return 1;
            }
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {*imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(vwo->commandBuffers->at(imageIndex));

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

        VkSwapchainKHR swapChains[] = {*(vwo->swapchain)};
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