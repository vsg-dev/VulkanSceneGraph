#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <any>

#include <vsg/ui/UIEvent.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/Semaphore.h>
#include <vsg/vk/Stage.h>

namespace vsg
{

    class VSG_DECLSPEC Window : public Inherit<Object, Window>
    {
    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        class Traits : public Inherit<Object, Traits>
        {
        public:
            Traits() {}
            Traits(const Traits&) = delete;
            Traits& operator=(const Traits&) = delete;

            Traits(int32_t in_x, int32_t in_y, uint32_t in_width, uint32_t in_height) :
                x(in_x),
                y(in_y),
                width(in_width),
                height(in_height) {}

            Traits(uint32_t in_width, uint32_t in_height) :
                width(in_width),
                height(in_height) {}

            int32_t x = 0;
            int32_t y = 0;
            uint32_t width = 1280;
            uint32_t height = 1024;

            bool fullscreen = false;

            uint32_t screenNum = 0;

            std::string windowClass = "vsg::Window";
            std::string windowTitle = "vsg window";

            bool decoration = true;
            bool hdpi = true;

            // X11 hint of whether to ignore the Window managers redirection of window size/position
            bool overrideRedirect = false;

            bool debugLayer = false;
            bool apiDumpLayer = false;

            SwapchainPreferences swapchainPreferences;

            vsg::Names instanceExtensionNames;
            vsg::Names deviceExtensionNames;

            Window* shareWindow = nullptr;

            AllocationCallbacks* allocator = nullptr;

            std::any nativeHandle;
            void* nativeWindow;

        protected:
            virtual ~Traits() {}
        };

        using Result = vsg::Result<Window, VkResult, VK_SUCCESS>;
        static Result create(vsg::ref_ptr<Traits> traits);

        // for backward compatibility
        static Result create(uint32_t width, uint32_t height, bool debugLayer = false, bool apiDumpLayer = false, vsg::Window* shareWindow = nullptr, vsg::AllocationCallbacks* allocator = nullptr);
        static Result create(vsg::ref_ptr<Traits> traits, bool debugLayer, bool apiDumpLayer = false, vsg::AllocationCallbacks* allocator = nullptr);

        static vsg::Names getInstanceExtensions();

        virtual bool valid() const { return false; }

        virtual bool pollEvents(Events& /*events*/) { return false; }

        virtual bool resized() const { return false; }
        virtual void resize() {}

        using Stages = std::vector<ref_ptr<Stage>>;

        void addStage(ref_ptr<Stage> stage) { _stages.push_back(stage); }

        Stages& stages() { return _stages; }
        const Stages& stages() const { return _stages; }

        const VkExtent2D& extent2D() { return _extent2D; }

        VkClearColorValue& clearColor() { return _clearColor; }
        const VkClearColorValue& clearColor() const { return _clearColor; }

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

        VkResult acquireNextImage(uint64_t timeout, VkSemaphore samaphore, VkFence fence)
        {
            return vkAcquireNextImageKHR(*_device, *_swapchain, timeout, samaphore, fence, &_nextImageIndex);
        }

        VkResult acquireNextImage(uint64_t timeout = std::numeric_limits<uint64_t>::max())
        {
            return vkAcquireNextImageKHR(*_device, *_swapchain, timeout, *(_frames[_nextImageIndex].imageAvailableSemaphore), VK_NULL_HANDLE, &_nextImageIndex);
        }

        uint32_t nextImageIndex() const { return _nextImageIndex; }

        void advanceNextImageIndex() { _nextImageIndex = (_nextImageIndex + 1) % _frames.size(); }

        bool debugLayersEnabled() const { return _traits->debugLayer; }

        void populateCommandBuffers(uint32_t index, ref_ptr<vsg::FrameStamp> frameStamp);

        struct Frame
        {
            // do we need a imageAvailableSemaphore per Frame? Probably..
            ref_ptr<Semaphore> imageAvailableSemaphore;

            ref_ptr<ImageView> imageView;
            ref_ptr<Framebuffer> framebuffer;

            // should we have multiple commandPool and commandBuffer?
            ref_ptr<CommandPool> commandPool;
            ref_ptr<CommandBuffer> commandBuffer;

            bool checkCommandsCompletedFence = false;
            ref_ptr<Fence> commandsCompletedFence;
        };

        using Frames = std::vector<Frame>;

        Frame& frame(uint32_t i) { return _frames[i]; }
        Frames& frames() { return _frames; }

    protected:
        Window(vsg::ref_ptr<vsg::Window::Traits> traits, vsg::AllocationCallbacks* allocator);

        virtual ~Window();

        virtual void clear();
        void share(const Window& window);
        void initaliseDevice();
        void buildSwapchain(uint32_t width, uint32_t height);

        ref_ptr<Traits> _traits;

        VkExtent2D _extent2D;
        VkClearColorValue _clearColor;

        ref_ptr<Instance> _instance;
        ref_ptr<PhysicalDevice> _physicalDevice;
        ref_ptr<Device> _device;
        ref_ptr<Surface> _surface;
        ref_ptr<Swapchain> _swapchain;
        ref_ptr<RenderPass> _renderPass;
        ref_ptr<Image> _depthImage;
        ref_ptr<DeviceMemory> _depthImageMemory;
        ref_ptr<ImageView> _depthImageView;

        Frames _frames;
        uint32_t _nextImageIndex;

        Stages _stages;
    };

    using Windows = std::vector<ref_ptr<Window>>;

} // namespace vsg
