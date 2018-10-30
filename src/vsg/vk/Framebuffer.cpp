/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Framebuffer.h>

using namespace vsg;

Framebuffer::Framebuffer(VkFramebuffer framebuffer, Device* device, AllocationCallbacks* allocator) :
    _framebuffer(framebuffer),
    _device(device),
    _allocator(allocator)
{
}

Framebuffer::~Framebuffer()
{
    if (_framebuffer)
    {
        vkDestroyFramebuffer(*_device, _framebuffer, _allocator);
    }
}

Framebuffer::Result Framebuffer::create(Device* device, VkFramebufferCreateInfo& framebufferInfo, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::Framebuffer::create(...) failed to create Framebuffer, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer(*device, &framebufferInfo, allocator, &framebuffer);
    if (result == VK_SUCCESS)
    {
        return Result(new Framebuffer(framebuffer, device, allocator));
    }
    else
    {
        return Result("Error: vsg::Framebuffer::create(...) Failed to create VkFramebuffer.", result);
    }
}
