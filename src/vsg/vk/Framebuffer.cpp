/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/vk/Framebuffer.h>

using namespace vsg;

Framebuffer::Framebuffer(ref_ptr<RenderPass> renderPass, const ImageViews& attachments, uint32_t width, uint32_t height, uint32_t layers) :
    _device(renderPass->device),
    _renderPass(renderPass),
    _attachments(attachments),
    _width(width),
    _height(height),
    _layers(layers)
{
    auto deviceID = _device->deviceID;

    std::vector<VkImageView> vk_attachments;
    for (auto& attachment : attachments)
    {
        vk_attachments.push_back(attachment->vk(deviceID));
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags = 0;
    framebufferInfo.renderPass = *_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
    framebufferInfo.pAttachments = vk_attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = layers;

    if (VkResult result = vkCreateFramebuffer(*_device, &framebufferInfo, nullptr, &_framebuffer); result != VK_SUCCESS)
    {
        throw Exception{"Error: vsg::Framebuffer::create(...) Failed to create VkFramebuffer.", result};
    }
}

Framebuffer::~Framebuffer()
{
    if (_framebuffer)
    {
        vkDestroyFramebuffer(*_device, _framebuffer, nullptr);
    }
}
