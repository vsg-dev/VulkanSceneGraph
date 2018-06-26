#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{
    struct ImageView : public vsg::Object
    {
        vsg::ref_ptr<Device>    _device;
        VkImageView             _imageView;
        VkAllocationCallbacks*  _pAllocator;

        ImageView(Device* device, VkImageView imageView, VkAllocationCallbacks* pAllocator=nullptr);

        virtual ~ImageView();

        operator VkImageView() const { return _imageView; }
    };
}
