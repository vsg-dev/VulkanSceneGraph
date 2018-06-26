#pragma once

#include <vsg/vk/Device.h>

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
}
