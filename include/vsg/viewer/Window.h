#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/UIEvent.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/Semaphore.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/viewer/WindowTraits.h>

namespace vsg
{
    class VSG_DECLSPEC Window : public Inherit<Object, Window>
    {
    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        static ref_ptr<Window> create(vsg::ref_ptr<WindowTraits> traits);

        virtual const char* instanceExtensionSurfaceName() const = 0;

        virtual bool valid() const { return false; }

        virtual bool visible() const { return valid(); }

        /// events buffered since the last pollEvents.
        UIEvents bufferedEvents;

        /// get the list of events since the last poolEvents() call by splicing bufferEvents with polled windowing events.
        virtual bool pollEvents(UIEvents& events);

        virtual bool resized() const { return false; }
        virtual void resize() {}

        WindowTraits* traits() { return _traits.get(); }
        const WindowTraits* traits() const { return _traits.get(); }

        const VkExtent2D& extent2D() const { return _extent2D; }

        VkClearColorValue& clearColor() { return _clearColor; }
        const VkClearColorValue& clearColor() const { return _clearColor; }

        VkSurfaceFormatKHR surfaceFormat();

        VkFormat depthFormat();

        VkSampleCountFlagBits framebufferSamples() const { return _framebufferSamples; }

        void setInstance(ref_ptr<Instance> instance);
        Instance* getInstance() { return _instance; }
        Instance* getOrCreateInstance();

        void setSurface(ref_ptr<Surface> surface);
        Surface* getSurface() { return _surface; }
        Surface* getOrCreateSurface();

        void setPhysicalDevice(ref_ptr<PhysicalDevice> physicalDevice);
        PhysicalDevice* getPhysicalDevice() { return _physicalDevice; }
        PhysicalDevice* getOrCreatePhysicalDevice();

        void setDevice(ref_ptr<Device> device);
        Device* getDevice() { return _device; }
        Device* getOrCreateDevice();

        void setRenderPass(ref_ptr<RenderPass> renderPass);
        RenderPass* getRenderPass() { return _renderPass; }
        RenderPass* getOrCreateRenderPass();

        Swapchain* getSwapchain() { return _swapchain; }
        Swapchain* getOrCreateSwapchain();

        Image* getDepthImage() { return _depthImage; }
        Image* getOrCreateDepthImage();

        ImageView* getDepthImageView() { return _depthImageView; }
        ImageView* getOrCreateDepthImageView();

        size_t numFrames() const { return _frames.size(); }

        ImageView* imageView(size_t i) { return _frames[i].imageView; }
        const ImageView* imageView(size_t i) const { return _frames[i].imageView; }

        Framebuffer* framebuffer(size_t i) { return _frames[i].framebuffer; }
        const Framebuffer* framebuffer(size_t i) const { return _frames[i].framebuffer; }

        /// call vkAquireNextImageKHR to find the next imageIndex of the swapchain images/framebuffers
        VkResult acquireNextImage(uint64_t timeout = std::numeric_limits<uint64_t>::max());

        /// get the image index for specified relative frame index, a 0 value is the current frame being rendered, 1 is the previous frame, 2 is the previous frame that.
        size_t imageIndex(size_t relativeFrameIndex = 0) const { return relativeFrameIndex < _indices.size() ? _indices[relativeFrameIndex] : _indices.size(); }

        bool debugLayersEnabled() const { return _traits->debugLayer; }

        struct Frame
        {
            ref_ptr<ImageView> imageView;
            ref_ptr<Framebuffer> framebuffer;
            ref_ptr<Semaphore> imageAvailableSemaphore;
        };

        using Frames = std::vector<Frame>;

        Frame& frame(size_t i) { return _frames[i]; }
        Frames& frames() { return _frames; }

    protected:
        Window(ref_ptr<WindowTraits> traits);

        virtual ~Window();

        virtual void _initSurface() = 0;
        void _initFormats();
        void _initInstance();
        void _initDevice();
        void _initRenderPass();
        void _initSwapchain();

        virtual void clear();
        void share(Window& window);
        void buildSwapchain();

        ref_ptr<WindowTraits> _traits;

        VkExtent2D _extent2D;
        VkClearColorValue _clearColor;
        VkSurfaceFormatKHR _imageFormat;
        VkFormat _depthFormat;

        VkSampleCountFlagBits _framebufferSamples;

        ref_ptr<Instance> _instance;
        ref_ptr<PhysicalDevice> _physicalDevice;
        ref_ptr<Device> _device;
        ref_ptr<Surface> _surface;
        ref_ptr<Swapchain> _swapchain;
        ref_ptr<RenderPass> _renderPass;
        ref_ptr<Image> _depthImage;
        ref_ptr<DeviceMemory> _depthImageMemory;
        ref_ptr<ImageView> _depthImageView;

        // only used when multisampling is required
        ref_ptr<Image> _multisampleImage;
        ref_ptr<ImageView> _multisampleImageView;

        ref_ptr<Semaphore> _availableSemaphore;

        Frames _frames;
        std::vector<size_t> _indices;
    };
    VSG_type_name(vsg::Window);

    using Windows = std::vector<ref_ptr<Window>>;

} // namespace vsg
