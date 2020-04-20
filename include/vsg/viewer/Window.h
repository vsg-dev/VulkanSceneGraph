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

#include <vsg/viewer/WindowTraits.h>

namespace vsg
{
    class VSG_DECLSPEC Window : public Inherit<Object, Window>
    {
    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        using Result = vsg::Result<Window, VkResult, VK_SUCCESS>;
        static Result create(vsg::ref_ptr<WindowTraits> traits);

        // for backward compatibility
        static Result create(uint32_t width, uint32_t height, bool debugLayer = false, bool apiDumpLayer = false, vsg::Window* shareWindow = nullptr, vsg::AllocationCallbacks* allocator = nullptr);
        static Result create(vsg::ref_ptr<WindowTraits> traits, bool debugLayer, bool apiDumpLayer = false, vsg::AllocationCallbacks* allocator = nullptr);

        static vsg::Names getInstanceExtensions();

        virtual bool valid() const { return false; }

        virtual bool pollEvents(Events& /*events*/) { return false; }

        virtual bool resized() const { return false; }
        virtual void resize() {}

        WindowTraits* traits() { return _traits.get(); }
        const WindowTraits* traits() const { return _traits.get(); }

        const VkExtent2D& extent2D() const { return _extent2D; }

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

        struct Frame
        {
            ref_ptr<ImageView> imageView;
            ref_ptr<Framebuffer> framebuffer;

            ref_ptr<Semaphore> imageAvailableSemaphore;
            ref_ptr<Fence> commandsCompletedFence;
        };

        using Frames = std::vector<Frame>;

        Frame& frame(uint32_t i) { return _frames[i]; }
        Frames& frames() { return _frames; }

    protected:
        Window(ref_ptr<WindowTraits> traits, AllocationCallbacks* allocator);

        virtual ~Window();

        virtual void clear();
        void share(const Window& window);
        void initaliseDevice();
        void buildSwapchain(uint32_t width, uint32_t height);

        ref_ptr<WindowTraits> _traits;

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
    };

    using Windows = std::vector<ref_ptr<Window>>;

} // namespace vsg
