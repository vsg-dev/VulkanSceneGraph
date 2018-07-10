#include <vsg/vk/ImageView.h>

#include <iostream>

namespace vsg
{

ImageView::ImageView(Device* device, VkImageView imageView, AllocationCallbacks* allocator) :
    _device(device),
    _imageView(imageView),
    _allocator(allocator)
{
}

ImageView::~ImageView()
{
    if (_imageView)
    {
        std::cout<<"Calling vkDestroyImageView(..)"<<std::endl;
        vkDestroyImageView(*_device, _imageView, *_allocator);
    }
}

}
