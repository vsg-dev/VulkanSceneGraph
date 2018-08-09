#pragma once

#include <vsg/vk/Image.h>

namespace vsg
{
    class ImageView : public Object
    {
    public:
        ImageView(VkImageView imageView, Device* device, Image* image=nullptr, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<ImageView, VkResult, VK_SUCCESS>;

        static Result create(Device* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, AllocationCallbacks* allocator=nullptr);

        static Result create(Device* device, Image* image, VkFormat format, VkImageAspectFlags aspectFlags, AllocationCallbacks* allocator=nullptr);

        operator VkImageView() const { return _imageView; }

    protected:

        virtual ~ImageView();

        VkImageView                     _imageView;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
        ref_ptr<Image>                  _image;

    };
}
