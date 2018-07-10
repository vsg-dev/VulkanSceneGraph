#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{
    struct ImageView : public vsg::Object
    {
        vsg::ref_ptr<Device>                _device;
        VkImageView                         _imageView;
        vsg::ref_ptr<AllocationCallbacks>  _allocator;

        ImageView(Device* device, VkImageView imageView, AllocationCallbacks* allocator=nullptr);

        virtual ~ImageView();

        operator VkImageView() const { return _imageView; }
    };
}
