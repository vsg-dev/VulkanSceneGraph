#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>

namespace vsg
{

    using AttachmentDescription = VkAttachmentDescription;

    using AttachmentReference = VkAttachmentReference;

    struct SubpassDescription
    {
        VkSubpassDescriptionFlags flags = 0;
        VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        std::vector<AttachmentReference> inputAttachments;
        std::vector<AttachmentReference> colorAttachments;
        std::vector<AttachmentReference> resolveAttachments;
        std::vector<AttachmentReference> depthStencilAttachments;
        std::vector<uint32_t> preserveAttachments;
    };

    using SubpassDependency = VkSubpassDependency;

    class VSG_DECLSPEC RenderPass : public Inherit<Object, RenderPass>
    {
    public:
        using Attachments = std::vector<AttachmentDescription>;
        using Subpasses = std::vector<SubpassDescription>;
        using Dependencies = std::vector<SubpassDependency>;

        RenderPass(Device* device, const Attachments& attachments, const Subpasses& subpasses, const Dependencies& dependencies);

        operator VkRenderPass() const { return _renderPass; }

        /// return the maximum VkAttachmentDescription.samples value of the assigned attachments.
        /// Used for be deciding if multisampling is required and the value to use when setting up the GraphicsPipeline's vsg::MultisampleState
        VkSampleCountFlagBits maxSamples() const { return _maxSamples; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~RenderPass();

        VkRenderPass _renderPass;
        VkSampleCountFlagBits _maxSamples;

        ref_ptr<Device> _device;
    };
    VSG_type_name(vsg::RenderPass);

    extern VSG_DECLSPEC AttachmentDescription defaultColorAttachment(VkFormat imageFormat);
    extern VSG_DECLSPEC AttachmentDescription defaultDepthAttachment(VkFormat depthFormat);

    extern VSG_DECLSPEC ref_ptr<RenderPass> createRenderPass(Device* device, VkFormat imageFormat, VkFormat depthFormat);
    extern VSG_DECLSPEC ref_ptr<RenderPass> createMultisampledRenderPass(Device* device, VkFormat imageFormat, VkFormat depthFormat,
                                                                         VkSampleCountFlagBits samples);

} // namespace vsg
