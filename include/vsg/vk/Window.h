#pragma once

#include <vsg/vk/CommandPool.h>

namespace vsg
{


    class Window : public Object
    {
    public:

        Window(const Window&) = delete;
        Window& operator = (const Window&) = delete;

        virtual bool valid() const { return false; }

        virtual bool resized() const { return false; }
        virtual void resize() {}

        const VkExtent2D& extent2D() { return _extent2D; }

        Instance* instance() { return _instance; }
        const Instance* instance() const { return _instance; }

        PhysicalDevice* physicalDevice() { return _physicalDevice; }
        const PhysicalDevice* physicalDevice() const { return _physicalDevice; }

        Device* device() { return _device; }
        const Device* device() const { return _device; }

        Surface* surface() { return _surface; }
        const Surface* surface() const { return _surface; }

        Swapchain* swapchain() { return _swapchain; }
        const Swapchain* swapchain() const { return _swapchain; }

        size_t numFrames() const { return _frames.size(); }

        ImageView* imageView(size_t i) { return _frames[i].imageView; }
        const ImageView* imageView(size_t i) const { return _frames[i].imageView; }

        Framebuffer* framebuffer(size_t i) { return _frames[i].framebuffer; }
        const Framebuffer* framebuffer(size_t i) const { return _frames[i].framebuffer; }

        CommandPool* commandPool(size_t i) { return _frames[i].commandPool; }
        const CommandPool* commandPool(size_t i) const { return _frames[i].commandPool; }

        CommandBuffer* commandBuffer(size_t i) { return _frames[i].commandBuffer; }
        const CommandBuffer* commandBuffer(size_t i) const { return _frames[i].commandBuffer; }


        VkResult acquireNextImage(uint64_t timeout, VkSemaphore samaphore, VkFence fence, uint32_t* imageIndex)
        {
            return vkAcquireNextImageKHR(*_device, *_swapchain, timeout, samaphore, fence, imageIndex);
        }

        void populateCommandBuffers(vsg::Node* commandGraph);

    protected:

        Window();

        virtual ~Window();

        virtual void clear();
        void share(const Window& window);
        void buildSwapchain(uint32_t width, uint32_t height);

        struct Frame
        {
            ref_ptr<ImageView>      imageView;
            ref_ptr<Framebuffer>    framebuffer;
            ref_ptr<CommandPool>    commandPool;
            ref_ptr<CommandBuffer>  commandBuffer;

        };

        using Frames = std::vector<Frame>;

        VkExtent2D              _extent2D;

        ref_ptr<Instance>       _instance;
        ref_ptr<PhysicalDevice> _physicalDevice;
        ref_ptr<Device>         _device;
        ref_ptr<Surface>        _surface;
        ref_ptr<Swapchain>      _swapchain;
        ref_ptr<RenderPass>     _renderPass;

        Frames                  _frames;
    };


}
