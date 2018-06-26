#include <vsg/vk/ImageView.h>

#include <iostream>

namespace vsg
{

ImageView::ImageView(Device* device, VkImageView imageView, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _imageView(imageView),
    _pAllocator(pAllocator)
{
}

ImageView::~ImageView()
{
    if (_imageView)
    {
        std::cout<<"Calling vkDestroyImageView(..)"<<std::endl;
        vkDestroyImageView(*_device, _imageView, _pAllocator);
    }
}

}
