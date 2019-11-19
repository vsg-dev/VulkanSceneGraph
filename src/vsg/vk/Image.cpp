/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Image.h>

using namespace vsg;

Image::Image(VkImage image, Device* device, AllocationCallbacks* allocator) :
    _image(image),
    _device(device),
    _allocator(allocator)
{
}

Image::~Image()
{
    if (_deviceMemory)
    {
        _deviceMemory->release(_memoryOffset, 0); // TODO, we don't locally have a size allocated
    }

    if (_image)
    {
        vkDestroyImage(*_device, _image, _allocator);
    }
}

Image::Result Image::create(Device* device, const VkImageCreateInfo& createImageInfo, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::Image::create(...) failed to create vkImage, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkImage image;
    VkResult result = vkCreateImage(*device, &createImageInfo, allocator, &image);
    if (result == VK_SUCCESS)
    {
        return Result(new Image(image, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create vkImage.", result);
    }
}
