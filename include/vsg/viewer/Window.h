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
#include <vsg/viewer/FrameAssembly.h>

namespace vsg
{
    class VSG_DECLSPEC Window : public Inherit<FrameAssembly, Window>
    {
    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        static ref_ptr<Window> create(vsg::ref_ptr<WindowTraits> traits);

        virtual const char* instanceExtensionSurfaceName() const = 0;

        virtual bool valid() const { return false; }

        virtual bool visible() const { return valid(); }

        virtual bool pollEvents(Events& /*events*/) { return false; }

        virtual bool resized() const { return false; }
        virtual void resize() {}

        WindowTraits* traits() { return _traits.get(); }
        const WindowTraits* traits() const { return _traits.get(); }

        const VkExtent2D& extent2D() const { return _extent2D; }

        VkClearColorValue& clearColor() { return _clearColor; }
        const VkClearColorValue& clearColor() const { return _clearColor; }

        Instance* getInstance() { return _instance; }
        Instance* getOrCreateInstance()
        {
            if (!_instance) _initInstance();
            return _instance;
        }

        Surface* getSurface() { return _surface; }
        Surface* getOrCreateSurface()
        {
            if (!_surface) _initSurface();
            return _surface;
        }

        Device* getDevice() { return _device; }
        Device* getOrCreateDevice()
        {
            if (!_device) _initDevice();
            return _device;
        }

        PhysicalDevice* getPhysicalDevice() { return _physicalDevice; }
        PhysicalDevice* getOrCreatePhysicalDevice()
        {
            if (!_physicalDevice) _initDevice();
            return _physicalDevice;
        }

        void setRenderPass(RenderPass* renderPass) { _renderPass = renderPass; }
        RenderPass* getRenderPass() { return _renderPass; }
        RenderPass* getOrCreateRenderPass()
        {
            if (!_renderPass) _initRenderPass();
            return _renderPass;
        }

        Swapchain* getSwapchain() { return _swapchain; }
        Swapchain* getOrCreateSwapchain()
        {
            if (!_swapchain) _initSwapchain();
            return _swapchain;
        }

        size_t numFrames() const { return _frames.size(); }

        ImageView* imageView(size_t i) { return _frames[i].imageView; }
        const ImageView* imageView(size_t i) const { return _frames[i].imageView; }

        Framebuffer* framebuffer(size_t i) { return _frames[i].framebuffer; }
        const Framebuffer* framebuffer(size_t i) const { return _frames[i].framebuffer; }

        VkResult acquireNextImage(uint64_t timeout = std::numeric_limits<uint64_t>::max())
        {
            if (!_swapchain) _initSwapchain();
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
        };

        using Frames = std::vector<Frame>;

        Frame& frame(uint32_t i) { return _frames[i]; }
        Frames& frames() { return _frames; }

        FrameAssembly::FrameRender getFrameRender() override;
        ref_ptr<Device> getDevice() const override;
        const VkExtent2D& getExtent2D() const override;

    protected:
        Window(ref_ptr<WindowTraits> traits);

        virtual ~Window();

        virtual void _initSurface() = 0;
        void _initInstance();
        void _initDevice();
        void _initRenderPass();
        void _initSwapchain();

        virtual void clear();
        void share(const Window& window);
        void buildSwapchain();

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
    VSG_type_name(vsg::Window);

    using Windows = std::vector<ref_ptr<Window>>;

} // namespace vsg
