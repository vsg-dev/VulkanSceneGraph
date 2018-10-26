#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandPool.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/Semaphore.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/State.h>

namespace vsg
{

    class VSG_DECLSPEC Window : public Inherit<Object, Window>
    {
    public:

        Window(const Window&) = delete;
        Window& operator = (const Window&) = delete;

        using Result = vsg::Result<Window, VkResult, VK_SUCCESS>;
        static Result create(uint32_t width, uint32_t height, bool debugLayer=false, bool apiDumpLayer=false, Window* shareWindow=nullptr, AllocationCallbacks* allocator=nullptr);

        virtual bool valid() const { return false; }

        virtual bool pollEvents() { return false; }

        virtual bool resized() const { return false; }
        virtual void resize() {}


        using Stages = std::vector<ref_ptr<Stage>>;
        Stages _stages;

        void addStage(ref_ptr<Stage> stage) { _stages.push_back(stage); }


        const VkExtent2D& extent2D() { return _extent2D; }

        Instance* instance() { return _instance; }
        const Instance* instance() const { return _instance; }

        PhysicalDevice* physicalDevice() { return _physicalDevice; }
        const PhysicalDevice* physicalDevice() const { return _physicalDevice; }

        Device* device() { return _device; }
        const Device* device() const { return _device; }

        Surface* surface() { return _surface; }
        const Surface* surface() const { return _surface; }

        RenderPass* renderPass() { return _renderPass; }
        const RenderPass* renderPass() const { return _renderPass; }

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

        Semaphore* imageAvailableSemaphore() { return _imageAvailableSemaphore; }
        const Semaphore* imageAvailableSemaphore() const { return _imageAvailableSemaphore; }

        VkResult acquireNextImage(uint64_t timeout, VkSemaphore samaphore, VkFence fence)
        {
            return vkAcquireNextImageKHR(*_device, *_swapchain, timeout, samaphore, fence, &_nextImageIndex);
        }

        uint32_t nextImageIndex() const { return _nextImageIndex; }

        bool debugLayersEnabled() const { return _debugLayersEnabled; }

        void populateCommandBuffers();

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
        VkClearColorValue       _clearColor;

        ref_ptr<Instance>       _instance;
        ref_ptr<PhysicalDevice> _physicalDevice;
        ref_ptr<Device>         _device;
        ref_ptr<Surface>        _surface;
        ref_ptr<Swapchain>      _swapchain;
        ref_ptr<RenderPass>     _renderPass;
        ref_ptr<Image>          _depthImage;
        ref_ptr<DeviceMemory>   _depthImageMemory;
        ref_ptr<ImageView>      _depthImageView;

        ref_ptr<Semaphore>      _imageAvailableSemaphore;

        Frames                  _frames;

        bool                    _debugLayersEnabled;
        uint32_t                _nextImageIndex;
    };


}
