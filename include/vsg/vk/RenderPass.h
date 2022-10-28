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
    /// AttachmentDescription is used by RenderPass to specify VkAttachmentDescription settings
    struct AttachmentDescription
    {
        VkAttachmentDescriptionFlags flags;
        VkFormat format;
        VkSampleCountFlagBits samples;
        VkAttachmentLoadOp loadOp;
        VkAttachmentStoreOp storeOp;
        VkAttachmentLoadOp stencilLoadOp;
        VkAttachmentStoreOp stencilStoreOp;
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;
    };
    VSG_type_name(vsg::AttachmentDescription);

    /// SubpassDependency is used by RenderPass to specify VkSubpassDependency settings
    struct SubpassDependency
    {
        uint32_t srcSubpass = 0;
        uint32_t dstSubpass = 0;
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
        VkDependencyFlags dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        /// multiview support requires Vulkan 1.2 or later.
        int32_t viewOffset = 0;
    };
    VSG_type_name(vsg::SubpassDependency);

    /// AttachmentReference is used by RenderPass to specify VkAttachmentReference settings
    struct AttachmentReference
    {
        uint32_t attachment = 0;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

        /// multiview support requires Vulkan 1.2 or later.
        VkImageAspectFlags aspectMask = 0;
    };
    VSG_type_name(vsg::AttachmentReference);

    /// SubpassDescription is used by RenderPass to specify VkSubpassDescription settings
    struct SubpassDescription
    {
        VkSubpassDescriptionFlags flags = 0;
        VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        std::vector<AttachmentReference> inputAttachments;
        std::vector<AttachmentReference> colorAttachments;
        std::vector<AttachmentReference> resolveAttachments;
        std::vector<AttachmentReference> depthStencilAttachments;

        std::vector<uint32_t> preserveAttachments;

        /// multiview support requires Vulkan 1.2 or later.
        uint32_t viewMask = 0;

        /// maps to VkSubpassDescriptionDepthStencilResolve, requires Vulkan 1.2 or later.
        VkResolveModeFlagBits depthResolveMode = VK_RESOLVE_MODE_NONE;
        VkResolveModeFlagBits stencilResolveMode = VK_RESOLVE_MODE_NONE;
        std::vector<AttachmentReference> depthStencilResolveAttachments;
    };
    VSG_type_name(vsg::SubpassDescription);

    /// RenderPass encapsulation of VkRenderPass
    class VSG_DECLSPEC RenderPass : public Inherit<Object, RenderPass>
    {
    public:
        using Attachments = std::vector<AttachmentDescription>;
        using Subpasses = std::vector<SubpassDescription>;
        using Dependencies = std::vector<SubpassDependency>;
        using CorrelatedViewMasks = std::vector<uint32_t>;

        RenderPass(Device* in_device, const Attachments& in_attachments, const Subpasses& in_subpasses, const Dependencies& in_dependencies, const CorrelatedViewMasks& in_correlatedViewMasks = {});

        operator VkRenderPass() const { return _renderPass; }
        VkRenderPass vk() const { return _renderPass; }

        // configuration of RenderPass used when creating vkRenderPass
        const ref_ptr<Device> device;
        const Attachments attachments;
        const Subpasses subpasses;
        const Dependencies dependencies;
        const CorrelatedViewMasks correlatedViewMasks;

        /// Maximum VkAttachmentDescription.samples value of the assigned attachments.
        /// Used for be deciding if multisampling is required and the value to use when setting up the GraphicsPipeline's vsg::MultisampleState
        const VkSampleCountFlagBits maxSamples;

    protected:
        virtual ~RenderPass();

        /// Vulkan renderPass handle, created in RenderPass constructor.
        VkRenderPass _renderPass;
    };
    VSG_type_name(vsg::RenderPass);

    extern VSG_DECLSPEC AttachmentDescription defaultColorAttachment(VkFormat imageFormat);
    extern VSG_DECLSPEC AttachmentDescription defaultDepthAttachment(VkFormat depthFormat);

    extern VSG_DECLSPEC ref_ptr<RenderPass> createRenderPass(Device* device, VkFormat imageFormat, VkFormat depthFormat, bool requiresDepthRead = false);
    extern VSG_DECLSPEC ref_ptr<RenderPass> createMultisampledRenderPass(Device* device, VkFormat imageFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, bool requiresDepthRead = false);

} // namespace vsg
