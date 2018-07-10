#pragma once

#include <vsg/vk/Surface.h>
#include <vsg/vk/ImageView.h>

namespace vsg
{
    class Swapchain : public vsg::Object
    {
    public:
        Swapchain(Device* device, Surface* surface, VkSwapchainKHR swapchain, AllocationCallbacks*  allocator=nullptr);

        using Result = vsg::Result<Swapchain, VkResult, VK_SUCCESS>;
        static Result create(PhysicalDevice* physicalDevice, Device* device, Surface* surface, uint32_t width, uint32_t height, AllocationCallbacks*  allocator=nullptr);

        operator VkSwapchainKHR() const { return _swapchain; }

        VkFormat getImageFormat() const { return _format; }

        const VkExtent2D& getExtent() const { return _extent; }

        using ImageViews = std::vector<ref_ptr<ImageView>>;
        ImageViews& getImageViews() { return _imageViews; }
        const ImageViews& getImageViews() const { return _imageViews; }

    protected:

        virtual ~Swapchain();

        vsg::ref_ptr<Device>                _device;
        vsg::ref_ptr<Surface>               _surface;
        VkSwapchainKHR                      _swapchain;
        VkFormat                            _format;
        VkExtent2D                          _extent;
        ImageViews                          _imageViews;

        vsg::ref_ptr<AllocationCallbacks>   _allocator;

    };
}
