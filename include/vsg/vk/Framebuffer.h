#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/ImageView.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{

    class VSG_DECLSPEC Framebuffer : public Inherit<Object, Framebuffer>
    {
    public:
        Framebuffer(ref_ptr<RenderPass> renderPass, const ImageViews& attachments, uint32_t width, uint32_t height, uint32_t layers);

        operator VkFramebuffer() const { return _framebuffer; }
        VkFramebuffer vk() const { return _framebuffer; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        RenderPass* getRenderPass() { return _renderPass; }
        const RenderPass* getRenderPass() const { return _renderPass; }

        ImageViews& getAttachments() { return _attachments; }
        const ImageViews& getAttachments() const { return _attachments; }

        uint32_t width() const { return _width; }
        uint32_t height() const { return _height; }
        uint32_t layers() const { return _layers; }

    protected:
        virtual ~Framebuffer();

        VkFramebuffer _framebuffer;
        ref_ptr<Device> _device;

        ref_ptr<RenderPass> _renderPass;
        ImageViews _attachments;

        const uint32_t _width;
        const uint32_t _height;
        const uint32_t _layers;
    };
    VSG_type_name(vsg::Framebuffer);

} // namespace vsg
