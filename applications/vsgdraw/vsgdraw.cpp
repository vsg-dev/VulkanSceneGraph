#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/utils/CommandLine.h>

#include <vsg/nodes/Group.h>

#include <vsg/maths/transform.h>

#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/Pipeline.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/Image.h>
#include <vsg/vk/Sampler.h>

#include <vsg/viewer/Window.h>
#include <vsg/viewer/Viewer.h>

#include <osg/ImageUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

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
            vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(_instance, _surface);
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
            _physicalDevice = vsg::PhysicalDevice::create(_instance, _surface);
            _device = vsg::Device::create(_instance, _physicalDevice, validatedNames, deviceExtensions);

            // set up renderpass with the imageFormat that the swap chain will use
            vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*_physicalDevice, *_surface);
            VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
            _renderPass = vsg::RenderPass::create(_device, imageFormat.format);
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

    template<typename T, VkStructureType type>
    class Info : public Object, public T
    {
    public:
        Info() : T{type} {}

    protected:
        virtual ~Info() {}

    };

    using RenderPassBeginInfo = Info<VkRenderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO>;

    template<typename F>
    void dispatchCommandsToQueue(Device* device, CommandPool* commandPool, VkQueue queue, F function)
    {
        vsg::ref_ptr<vsg::CommandBuffer> transferCommand = vsg::CommandBuffer::create(device, commandPool, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = transferCommand->flags();

        vkBeginCommandBuffer(*transferCommand, &beginInfo);

            function(*transferCommand);

        vkEndCommandBuffer(*transferCommand);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = transferCommand->data();

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        // we must wait for the queue to empty before we can safely clean up the transferCommand
        vkQueueWaitIdle(queue);
    }


    class BufferChain : public Object
    {
    public:
        BufferChain(PhysicalDevice* physicalDevice, Device* device, VkBufferUsageFlags usage, VkSharingMode sharingMode):
            _physicalDevice(physicalDevice),
            _device(device),
            _usage(usage),
            _sharingMode(sharingMode)
        {
            if (usage==VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) _alignment = physicalDevice->getProperties().limits.minUniformBufferOffsetAlignment;
            else _alignment = 1;

            std::cout<<"BufferChain(... usage="<<usage<<"...) _alignment="<<_alignment<<std::endl;
        }

        VkDeviceSize dataSize() const
        {
            if (_entries.empty()) return 0;
            else return _entries.back().offset + _entries.back().data->dataSize();
        }

        void allocate(bool useStagingBuffer=true)
        {
            // copy the vertex data to a stageing buffer, then submit a command to copy it to the final vertex buffer hosted in GPU local memory.
            VkDeviceSize totalSize = dataSize();

            if (useStagingBuffer)
            {
                _stagingBuffer = vsg::Buffer::create(_device, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, _sharingMode);
                _stagingMemory = vsg::DeviceMemory::create(_physicalDevice, _device, _stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vkBindBufferMemory(*_device, *_stagingBuffer, *_stagingMemory, 0);

                _deviceBuffer = vsg::Buffer::create(_device, totalSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | _usage, _sharingMode);
                _deviceMemory =  vsg::DeviceMemory::create(_physicalDevice, _device, _deviceBuffer,  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                vkBindBufferMemory(*_device, *_deviceBuffer, *_deviceMemory, 0);

            }
            else
            {
                _deviceBuffer = vsg::Buffer::create(_device, totalSize, _usage, _sharingMode);
                _deviceMemory =  vsg::DeviceMemory::create(_physicalDevice, _device, _deviceBuffer,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vkBindBufferMemory(*_device, *_deviceBuffer, *_deviceMemory, 0);
            }
        }

        void transfer(CommandPool* commandPool, VkQueue graphicsQueue)
        {
            if (_stagingMemory)
            {
                VkDeviceSize totalSize = dataSize();

                std::cout<<"BufferChain::transfer() to device local memory using staging buffer, totalSize="<<totalSize<<std::endl;

                copy(_stagingBuffer, _stagingMemory);

                dispatchCommandsToQueue(_device, commandPool, graphicsQueue, [&](VkCommandBuffer transferCommand)
                {
                    VkBufferCopy copyRegion = {};
                    copyRegion.srcOffset = 0;
                    copyRegion.dstOffset = 0;
                    copyRegion.size = totalSize;
                    vkCmdCopyBuffer(transferCommand, *_stagingBuffer, *_deviceBuffer, 1, &copyRegion);
                });
            }
            else
            {
                //std::cout<<"BufferChain::transfer() copying to host visible memory, totalSize="<<totalSize<<std::endl;
                copy(_deviceBuffer, _deviceMemory);
            }
        }

        void copy(Buffer* buffer, DeviceMemory* memory)
        {
            char* buffer_data;
            vkMapMemory(*_device, *memory, 0, dataSize(), 0, (void**)(&buffer_data));

            char* ptr = (char*)(buffer_data);

            for(auto entry : _entries)
            {
                std::memcpy(buffer_data+entry.offset, entry.data->dataPointer(), entry.data->dataSize());
            }

            vkUnmapMemory(*_device, *memory);
        }

        void add(Data* data)
        {
            if (_entries.empty())
            {
                _entries.push_back(Entry{data, 0, 0});
            }
            else
            {
                Entry& previous_entry = _entries.back();
                VkDeviceSize endOfPreviousEntry = previous_entry.offset+previous_entry.data->dataSize();
                VkDeviceSize offset = (_alignment==1 || (endOfPreviousEntry%_alignment)==0) ? endOfPreviousEntry : ((endOfPreviousEntry/_alignment)+1)*_alignment;
                _entries.push_back(Entry{data, 0, offset});
            }
        }

        void print(std::ostream& out)
        {
            out<<"BufferChain "<<_entries.size()<<" "<<dataSize()<<std::endl;
            for(auto entry : _entries)
            {
                out<<"    entry "<<entry.data.get()<<", modifiedCount="<<entry.modifiedCount<<", offset="<<entry.offset<<" size="<<entry.data->dataSize()<<std::endl;
            }
        }

        DescriptorBufferInfos getDescriptorBufferInfo()
        {
            if (!_deviceBuffer ||  _deviceBuffer->usage()!=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) return DescriptorBufferInfos();

            VkBuffer buffer = *_deviceBuffer;
            DescriptorBufferInfos infos;
            for(auto entry : _entries)
            {
                infos.push_back(VkDescriptorBufferInfo{buffer, entry.offset, entry.data->dataSize()});
            }
            return infos;
        }


        struct Entry
        {
            ref_ptr<Data> data;
            unsigned int  modifiedCount;
            VkDeviceSize  offset;
        };

        ref_ptr<PhysicalDevice> _physicalDevice;
        ref_ptr<Device>         _device;
        VkBufferUsageFlags      _usage;
        VkSharingMode           _sharingMode;
        VkDeviceSize            _alignment;

        using Entries = std::vector<Entry>;
        Entries _entries;

        ref_ptr<Buffer>         _stagingBuffer;
        ref_ptr<DeviceMemory>   _stagingMemory;

        ref_ptr<Buffer>         _deviceBuffer;
        ref_ptr<DeviceMemory>   _deviceMemory;

    protected:
        ~BufferChain() {}
    };


    template<class T>
    void add(T binding, BufferChain* chain)
    {
        for (auto entry : chain->_entries)
        {
            binding->add(chain->_deviceBuffer, entry.offset);
        }
    }


    class CmdBindDescriptorSets : public Command
    {
    public:

        CmdBindDescriptorSets(PipelineLayout* pipelineLayout, const DescriptorSets& descriptorSets) : _pipelineLayout(pipelineLayout), _descriptorSets(descriptorSets) {}

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_pipelineLayout, 0, _descriptorSets.size(), _descriptorSets.data(), 0, nullptr);
        }

    protected:
        virtual ~CmdBindDescriptorSets() {}

        ref_ptr<PipelineLayout> _pipelineLayout;
        DescriptorSets _descriptorSets;
    };


    class ImageMemoryBarrier : public Object, public VkImageMemoryBarrier
    {
    public:

        ImageMemoryBarrier(VkAccessFlags in_srcAccessMask=0, VkAccessFlags in_destAccessMask=0,
                            VkImageLayout in_oldLayout=VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout in_newLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                            Image* in_image=nullptr) :
                            VkImageMemoryBarrier{}
        {
            sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            oldLayout = in_oldLayout;
            newLayout = in_newLayout;
            srcAccessMask = in_srcAccessMask;
            dstAccessMask = in_destAccessMask;
            srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            image = *in_image;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;
        }


        void cmdPiplineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
        {
            std::cout<<"cmdPiplineBarrier("<<std::endl;
            std::cout<<"    sourceStage = "<<sourceStage<<std::endl;
            std::cout<<"    destinationStage="<<destinationStage<<std::endl;
            std::cout<<"    srcAccessMask = 0x"<<std::hex<<srcAccessMask<<std::endl;
            std::cout<<"    dstAccessMask = 0x"<<dstAccessMask<<std::endl;
            std::cout<<"    oldLayout = "<<std::dec<<oldLayout<<std::endl;
            std::cout<<"    newLayout = "<<newLayout<<std::endl;

            vkCmdPipelineBarrier(commandBuffer,
                                 sourceStage, destinationStage,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, static_cast<VkImageMemoryBarrier*>(this));
        }

    };


}

namespace osg2vsg
{
    using GLtoVkFormatMap = std::map<std::pair<GLenum, GLenum>, VkFormat>;
    static GLtoVkFormatMap s_GLtoVkFormatMap = {
        {{GL_UNSIGNED_BYTE, GL_ALPHA}, VK_FORMAT_R8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_LUMINANCE}, VK_FORMAT_R8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_LUMINANCE_ALPHA}, VK_FORMAT_R8G8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_RGB}, VK_FORMAT_R8G8B8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_RGBA}, VK_FORMAT_R8G8B8A8_UNORM}
    };

    VkFormat convertGLImageFormatToVulkan(GLenum dataType, GLenum pixelFormat)
    {
        auto itr = s_GLtoVkFormatMap.find({dataType,pixelFormat});
        if (itr!=s_GLtoVkFormatMap.end())
        {
            std::cout<<"convertGLImageFormatToVulkan("<<dataType<<", "<<pixelFormat<<") vkFormat="<<itr->second<<std::endl;
            return itr->second;
        }
        else
        {
            std::cout<<"convertGLImageFormatToVulkan("<<dataType<<", "<<pixelFormat<<") no match found."<<std::endl;
            return VK_FORMAT_UNDEFINED;
        }
    }

    struct WriteRow : public osg::CastAndScaleToFloatOperation
    {
        WriteRow(unsigned char* ptr) : _ptr(ptr) {}
        unsigned char* _ptr;

        inline void luminance(float l) { rgba(l, l, l, 1.0f); }
        inline void alpha(float a) { rgba(1.0f, 1.0f, 1.0f, a); }
        inline void luminance_alpha(float l,float a) { rgba(l, l, l, a); }
        inline void rgb(float r,float g,float b) { rgba(r, g, b, 1.0f); }
        inline void rgba(float r,float g,float b,float a)
        {
            (*_ptr++) = static_cast<unsigned char>(r*255.0);
            (*_ptr++) = static_cast<unsigned char>(g*255.0);
            (*_ptr++) = static_cast<unsigned char>(b*255.0);
            (*_ptr++) = static_cast<unsigned char>(a*255.0);
        }
    };

    vsg::ref_ptr<osg::Image> formatImage(osg::Image* image, GLenum pixelFormat)
    {
        vsg::ref_ptr<osg::Image> new_image = new osg::Image;
        new_image->allocateImage(image->s(), image->t(), image->r(), pixelFormat, GL_UNSIGNED_BYTE);

        // need to copy pixels from image to new_image;
        for(int r=0;r<image->r();++r)
        {
            for(int t=0;t<image->t();++t)
            {
                WriteRow operation(new_image->data(0, t, r));
                osg::readRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
            }
        }

        return new_image;
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


    // set up vertex arrays
    vsg::ref_ptr<vsg::vec3Array> vertices = new vsg::vec3Array(4);
    vsg::ref_ptr<vsg::vec3Array> colors = new vsg::vec3Array(4);
    vsg::ref_ptr<vsg::vec2Array> texcoords = new vsg::vec2Array(4);

    vertices->set(0, {-0.5f, -0.5f, 0.0f});
    vertices->set(1, {0.5f,  -0.5f, 0.25f});
    vertices->set(2, {0.5f , 0.5f, 0.0f});
    vertices->set(3, {-0.5f, 0.5f, 0.0f});

    colors->set(0, {1.0f, 0.0f, 0.0f});
    colors->set(1, {0.0f, 1.0f, 0.0f});
    colors->set(2, {0.0f, 0.0f, 1.0f});
    colors->set(3, {1.0f, 1.0f, 1.0f});

    texcoords->set(0, {0.0f, 0.0f});
    texcoords->set(1, {1.0f, 0.0f});
    texcoords->set(2, {1.0f, 1.0f});
    texcoords->set(3, {0.0f, 1.0f});

    vsg::ref_ptr<vsg::ushortArray> indices = new vsg::ushortArray(6);
    indices->set(0, 0);
    indices->set(1, 1);
    indices->set(2, 2);
    indices->set(3, 2);
    indices->set(4, 3);
    indices->set(5, 0);

    // set up uniforms
    using DataList = std::vector<vsg::ref_ptr<vsg::Data>>;
    DataList uniforms;

    vsg::ref_ptr<vsg::mat4Value> projMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> viewMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> modelMatrix = new vsg::mat4Value;

    uniforms.push_back(projMatrix);
    uniforms.push_back(viewMatrix);
    uniforms.push_back(modelMatrix);

    vsg::ref_ptr<vsg::BufferChain> vertexBufferChain = new vsg::BufferChain(physicalDevice, device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vertexBufferChain->add(vertices);
    vertexBufferChain->add(colors);
    vertexBufferChain->add(texcoords);
    vertexBufferChain->allocate(useStagingBuffer);
    vertexBufferChain->transfer(commandPool, graphicsQueue);

    vsg::ref_ptr<vsg::BufferChain> indexBufferChain = new vsg::BufferChain(physicalDevice, device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    indexBufferChain->add(indices);
    indexBufferChain->allocate(useStagingBuffer);
    indexBufferChain->transfer(commandPool, graphicsQueue);

    vsg::ref_ptr<vsg::BufferChain> uniformBufferChain = new vsg::BufferChain(physicalDevice, device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    uniformBufferChain->add(projMatrix);
    uniformBufferChain->add(viewMatrix);
    uniformBufferChain->add(modelMatrix);
    uniformBufferChain->allocate(false); // useStagingBuffer);
    //uniformBufferChain->transfer(device, commandPool, graphicsQueue);

    vertexBufferChain->print(std::cout);
    indexBufferChain->print(std::cout);
    uniformBufferChain->print(std::cout);

    //
    // set up texture image
    //
    osg::ref_ptr<osg::Image> osg_image = osgDB::readImageFile("textures/lz.rgb");
    if (!osg_image)
    {
        std::cout<<"Could not laod image"<<std::endl;
        return 1;
    }


    if(osg_image->getPixelFormat()!=GL_RGBA || osg_image->getDataType()!=GL_UNSIGNED_BYTE)
    {
        std::cout<<"Reformating osg::Image to GL_RGBA, before = "<<osg_image->getPixelFormat()<<std::endl;
        osg_image = osg2vsg::formatImage(osg_image, GL_RGBA);
        std::cout<<"Reformating osg::Image to GL_RGBA, after = "<<osg_image->getPixelFormat()<<", RGBA="<<GL_RGBA<<std::endl;
    }

    VkDeviceSize imageTotalSize = osg_image->getTotalSizeInBytesIncludingMipmaps();

    vsg::ref_ptr<vsg::Buffer> imageStagingBuffer = vsg::Buffer::create(device, imageTotalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::ref_ptr<vsg::DeviceMemory> imageStagingMemory = vsg::DeviceMemory::create(physicalDevice, device, imageStagingBuffer,
                                                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(*device, *imageStagingBuffer, *imageStagingMemory, 0);

    // copy image data to staging memory
    imageStagingMemory->copy(0, imageTotalSize, osg_image->data());

    std::cout<<"Creating imageStagingBuffer and memorory size = "<<imageTotalSize<<std::endl;


    VkFormat format =  VK_FORMAT_R8G8B8A8_UNORM; //                 osg2vsg::convertGLImageFormatToVulkan(osg_image->getDataType(), osg_image->getPixelFormat());

    std::cout<<"VK_FORMAT_R8G8B8A8_UNORM= "<<VK_FORMAT_R8G8B8A8_UNORM<<std::endl;
    std::cout<<"  osg2vsg : "<<osg2vsg::convertGLImageFormatToVulkan(osg_image->getDataType(), osg_image->getPixelFormat())<<std::endl;


    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = osg_image->r()>1 ? VK_IMAGE_TYPE_3D : (osg_image->t()>1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D);
    imageCreateInfo.extent.width = osg_image->s();
    imageCreateInfo.extent.height = osg_image->t();
    imageCreateInfo.extent.depth = osg_image->r();
    imageCreateInfo.mipLevels = osg_image->getNumMipmapLevels();
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = osg2vsg::convertGLImageFormatToVulkan(osg_image->getDataType(), osg_image->getPixelFormat());
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vsg::ref_ptr<vsg::Image> textureImage = vsg::Image::create(device, imageCreateInfo);
    if (!textureImage)
    {
        return 1;
    }

    vsg::ref_ptr<vsg::DeviceMemory> textureImageDeviceMemory = vsg::DeviceMemory::create(physicalDevice, device, textureImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!textureImageDeviceMemory)
    {
        return 1;
    }

    // should the textureImage take a reference to the textureImageDeviceMemory?  Possibly, will need to consider later.
    vkBindImageMemory(*device, *textureImage, *textureImageDeviceMemory, 0);


    vsg::dispatchCommandsToQueue(device, commandPool, graphicsQueue, [&](VkCommandBuffer commandBuffer)
    {

        std::cout<<"Need to dispatch VkCmd's to "<<commandBuffer<<std::endl;

        vsg::ImageMemoryBarrier preCopyImageMemoryBarrier(
                        0, VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        textureImage);

        preCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        std::cout<<"CopyBufferToImage()"<<std::endl;

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {static_cast<uint32_t>(osg_image->s()), static_cast<uint32_t>(osg_image->t()), 1};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        std::cout<<"Post CopyBufferToImage()"<<std::endl;

        vsg::ImageMemoryBarrier postCopyImageMemoryBarrier(
                        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        textureImage);

        postCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        std::cout<<"Post postCopyImageMemoryBarrier()"<<std::endl;
    });

    // clean up staging buffer
    imageStagingBuffer = 0;
    imageStagingMemory = 0;

    // delete osg_image as it's no longer required.
    osg_image = 0;

    vsg::ref_ptr<vsg::ImageView> textureImageView = vsg::ImageView::create(device, *textureImage, VK_FORMAT_R8G8B8A8_UNORM);

    // default texture sampler
    vsg::ref_ptr<vsg::Sampler> textureSampler = vsg::Sampler::create(device);




    //
    // set up descriptor set for uniforms
    //
    std::vector<VkDescriptorSetLayoutBinding> descriptorLayoutBinding(4);
    descriptorLayoutBinding[0].binding = 0;
    descriptorLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorLayoutBinding[0].descriptorCount = projMatrix->valueCount();
    descriptorLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorLayoutBinding[0].pImmutableSamplers = nullptr;

    descriptorLayoutBinding[1].binding = 1;
    descriptorLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorLayoutBinding[1].descriptorCount = viewMatrix->valueCount();
    descriptorLayoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorLayoutBinding[1].pImmutableSamplers = nullptr;

    descriptorLayoutBinding[2].binding = 2;
    descriptorLayoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorLayoutBinding[2].descriptorCount = modelMatrix->valueCount();
    descriptorLayoutBinding[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorLayoutBinding[2].pImmutableSamplers = nullptr;

    descriptorLayoutBinding[3].binding = 3;
    descriptorLayoutBinding[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorLayoutBinding[3].descriptorCount = 1;
    descriptorLayoutBinding[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorLayoutBinding[3].pImmutableSamplers = nullptr;


    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = descriptorLayoutBinding.size();
    layoutInfo.pBindings = descriptorLayoutBinding.data();

    vsg::ref_ptr<vsg::DescriptorSetLayout> descriptorSetLayout = vsg::DescriptorSetLayout::create(device, layoutInfo);


    vsg::ref_ptr<vsg::CmdBindVertexBuffers> bindVertexBuffers = new vsg::CmdBindVertexBuffers;
    vsg::add(bindVertexBuffers, vertexBufferChain);

    vsg::ref_ptr<vsg::CmdBindIndexBuffer> bindIndexBuffer = new vsg::CmdBindIndexBuffer(indexBufferChain->_deviceBuffer, 0, VK_INDEX_TYPE_UINT16);

    vsg::VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}
    };
    vsg::VertexInputState::Attributes vertexAttrobiteDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
        VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},
    };

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

    // set up renderpass with the imageFormat that the swap chain will use
    vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*physicalDevice, *surface);
    VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
    vsg::ref_ptr<vsg::RenderPass> renderPass = vsg::RenderPass::create(device, imageFormat.format);


    vsg::DescriptorPoolSizes poolSizes{
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
    };
    vsg::ref_ptr<vsg::DescriptorPool> descriptorPool = vsg::DescriptorPool::create(device, 1, poolSizes);

    VkDescriptorSetLayout descriptorSetLayouts[] = {*descriptorSetLayout};

    VkDescriptorSetAllocateInfo descriptSetAllocateInfo = {};
    descriptSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptSetAllocateInfo.descriptorSetCount = 1;
    descriptSetAllocateInfo.pSetLayouts = descriptorSetLayouts;

    vsg::DescriptorSets descriptorSets(1);
    if (vkAllocateDescriptorSets(*device, &descriptSetAllocateInfo, descriptorSets.data())!=VK_SUCCESS)
    {
        std::cout<<"Error: failed to create VkDescriptorSet"<<std::endl;
        return 1;
    }

    vsg::DescriptorBufferInfos descriptorBufferInfos = uniformBufferChain->getDescriptorBufferInfo();
    std::cout<<"uniformBufferChain->getDescriptorBufferInfo() "<<descriptorBufferInfos.size()<<std::endl;
    for (auto bufferInfo : descriptorBufferInfos)
    {
        std::cout<<"   VkDescriptorBufferInfo buffer="<<bufferInfo.buffer<<", offset="<<bufferInfo.offset<<", range="<<bufferInfo.range<<std::endl;
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites(2);
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets.front();
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = descriptorBufferInfos.size();
    descriptorWrites[0].pBufferInfo = descriptorBufferInfos.data();

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = *textureImageView;
    imageInfo.sampler = *textureSampler;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets.front();
    descriptorWrites[1].dstBinding = 3;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(*device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);


    // set up pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout =  vsg::PipelineLayout::create(device, pipelineLayoutInfo);

    // set up graphics pipeline
    vsg::ref_ptr<vsg::Pipeline> pipeline = vsg::createGraphicsPipeline(device, renderPass, pipelineLayout, pipelineStates);

    vsg::ref_ptr<vsg::CmdBindDescriptorSets> bindDescriptorSets = new vsg::CmdBindDescriptorSets(pipelineLayout, descriptorSets);

    // set up what we want to render

    //////////////////////////////////////////////////
    //
    //  Pipeline -> (device, renderPass, pipelineLayout, pipelineStates)
    //
    //          pipelioneStates ->  ShaderModules
    //                              VertexInputState
    //                              InputAssemblyState
    //                              RasterizationState
    //                              VertexInputState
    //                              MultisampleState
    //                              ColorBlendState
    //                              TessellationState
    //                              DepthStencilState
    //                              DynamicState
    //          RenderPass (dpenend upon imageFormat provided by Swapchain support))
    //          uint subpass
    //
    //          PipelineLayout ->   DescriptorSetLayouts (uniform bindings/stages)
    //
    //  CmdBindDescriptorSets (pass uniforms data)
    //
    //  CmdBindVertexBuffers (vertex arrays)
    //  CmdBindIndexBuffer (primitives indices)
    //
    //  CmdDrawInsdexed ispatch draw call
    //
    //////////////////////////////////////////////////



    // create command graph to contain all the Vulkan calls for specifically rendering the model
    vsg::ref_ptr<vsg::Group> commandGraph = new vsg::Group;

    // add renderPass to the command graph
    commandGraph->addChild(renderPass);

    // set up the state configuration
    renderPass->addChild(pipeline);
    renderPass->addChild(bindDescriptorSets);

    // add subgraph that represents the model to render
    vsg::ref_ptr<vsg::Group> model = new vsg::Group;
    renderPass->addChild(model);

    // add the vertex and index buffer data
    model->addChild(bindVertexBuffers);
    model->addChild(bindIndexBuffer);

    // add the draw primitive command
    model->addChild(drawIndexed);


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
        uniformBufferChain->transfer(commandPool, graphicsQueue);

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
