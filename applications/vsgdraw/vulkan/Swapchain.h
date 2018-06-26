#pragma once

#include "vulkan/Device.h"
#include "vulkan/Surface.h"

/////////////////////////////////////////////////////////////////////
//
// start of vulkan code
//
namespace vsg
{
    struct ImageView : public vsg::Object
    {
        vsg::ref_ptr<Device>    _device;
        VkImageView             _imageView;
        VkAllocationCallbacks*  _pAllocator;

        ImageView(Device* device, VkImageView imageView, VkAllocationCallbacks* pAllocator=nullptr):
            _device(device), _imageView(imageView), _pAllocator(pAllocator) {}

        virtual ~ImageView()
        {
            if (_imageView)
            {
                std::cout<<"Calling vkDestroyImageView(..)"<<std::endl;
                vkDestroyImageView(*_device, _imageView, _pAllocator);
            }
        }

        operator VkImageView() const { return _imageView; }
    };

    class Swapchain : public vsg::Object
    {
    public:
        Swapchain(Device* device, Surface* surface, VkSwapchainKHR swapchain, VkAllocationCallbacks*  pAllocator=nullptr):
            _device(device), _surface(surface), _swapchain(swapchain), _pAllocator(pAllocator) {}

        Swapchain(PhysicalDevice* physicalDevice, Device* device, Surface* surface, uint32_t width, uint32_t height, VkAllocationCallbacks*  pAllocator=nullptr);
        operator VkSwapchainKHR() const { return _swapchain; }

        using ImageViews = std::vector<ref_ptr<ImageView>>;
        ImageViews& getImageViews() { return _imageViews; }
        const ImageViews& getImageViews() const { return _imageViews; }

    protected:

        virtual ~Swapchain();

        vsg::ref_ptr<Device>    _device;
        vsg::ref_ptr<Surface>   _surface;
        VkSwapchainKHR          _swapchain;
        VkFormat                _format;
        VkExtent2D              _extent;
        ImageViews              _imageViews;

        VkAllocationCallbacks*  _pAllocator;

    };
}
