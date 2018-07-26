#pragma once

#include <vsg/vk/Image.h>

namespace vsg
{
    class ImageView : public vsg::Object
    {
    public:
        ImageView(Device* device, VkImageView imageView, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<ImageView, VkResult, VK_SUCCESS>;

        static Result create(Device* device, VkImage image, VkFormat format, AllocationCallbacks* allocator=nullptr);

        static Result create(Device* device, Image* image, VkFormat format, AllocationCallbacks* allocator=nullptr)
        {
            return create(device, *image, format, allocator);
        }

        operator VkImageView() const { return _imageView; }

    protected:

        virtual ~ImageView();

        vsg::ref_ptr<Device>                _device;
        VkImageView                         _imageView;
        vsg::ref_ptr<AllocationCallbacks>  _allocator;

    };
}
