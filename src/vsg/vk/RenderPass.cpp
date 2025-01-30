/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/ScratchMemory.h>
#include <vsg/io/Logger.h>
#include <vsg/vk/RenderPass.h>

#include <array>

using namespace vsg;

inline VkSampleCountFlagBits computeMaxSamples(const RenderPass::Attachments& attachments)
{
    VkSampleCountFlagBits maxSamples = VK_SAMPLE_COUNT_1_BIT;
    for (const auto& attachment : attachments)
    {
        if (attachment.samples > maxSamples) maxSamples = attachment.samples;
    }
    return maxSamples;
}

RenderPass::RenderPass(Device* in_device, const Attachments& in_attachments, const Subpasses& in_subpasses, const Dependencies& in_dependencies, const CorrelatedViewMasks& in_correlatedViewMasks) :
    device(in_device),
    attachments(in_attachments),
    subpasses(in_subpasses),
    dependencies(in_dependencies),
    correlatedViewMasks(in_correlatedViewMasks),
    maxSamples(computeMaxSamples(in_attachments))
{
    auto scratchMemory = ScratchMemory::create(1024);
    // vkCreateRenderPass2 is supported with the VK_KHR_create_renderpass2 device extension, or in Vulkan core 1.2 and later
    auto extensions = device->getExtensions();
    if (extensions->vkCreateRenderPass2)
    {
        // vkCreateRenderPass2 code path
        auto copyAttachmentDescriptions = [&scratchMemory](const Attachments& attachmentDescriptions) -> VkAttachmentDescription2* {
            if (attachmentDescriptions.empty()) return nullptr;

            auto vk_attachmentDescriptions = scratchMemory->allocate<VkAttachmentDescription2>(attachmentDescriptions.size());
            for (size_t i = 0; i < attachmentDescriptions.size(); ++i)
            {
                auto& src = attachmentDescriptions[i];
                auto& dst = vk_attachmentDescriptions[i];

                dst.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
                dst.pNext = nullptr;
                dst.flags = src.flags;
                dst.format = src.format;
                dst.samples = src.samples;
                dst.loadOp = src.loadOp;
                dst.storeOp = src.storeOp;
                dst.stencilLoadOp = src.stencilLoadOp;
                dst.stencilStoreOp = src.stencilStoreOp;
                dst.initialLayout = src.initialLayout;
                dst.finalLayout = src.finalLayout;
            }
            return vk_attachmentDescriptions;
        };

        auto copySubpasses = [&scratchMemory](const Subpasses& subpassDescriptions) -> VkSubpassDescription2* {
            if (subpassDescriptions.empty()) return nullptr;

            auto copyAttachmentReference = [&scratchMemory](const std::vector<AttachmentReference>& attachmentReferences) -> VkAttachmentReference2* {
                if (attachmentReferences.empty()) return nullptr;

                auto vk_attachments = scratchMemory->allocate<VkAttachmentReference2>(attachmentReferences.size());
                for (size_t i = 0; i < attachmentReferences.size(); ++i)
                {
                    auto& src = attachmentReferences[i];
                    auto& dst = vk_attachments[i];
                    dst.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
                    dst.pNext = nullptr;
                    dst.attachment = src.attachment;
                    dst.layout = src.layout;
                    dst.aspectMask = src.aspectMask;
                }
                return vk_attachments;
            };

            auto vk_subpassDescription = scratchMemory->allocate<VkSubpassDescription2>(subpassDescriptions.size());
            for (size_t i = 0; i < subpassDescriptions.size(); ++i)
            {
                auto& src = subpassDescriptions[i];
                auto& dst = vk_subpassDescription[i];
                dst.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
                dst.pNext = nullptr;
                dst.flags = src.flags;
                dst.pipelineBindPoint = src.pipelineBindPoint;
                dst.viewMask = src.viewMask;
                dst.inputAttachmentCount = static_cast<uint32_t>(src.inputAttachments.size());
                dst.pInputAttachments = copyAttachmentReference(src.inputAttachments);
                dst.colorAttachmentCount = static_cast<uint32_t>(src.colorAttachments.size());
                dst.pColorAttachments = copyAttachmentReference(src.colorAttachments);
                dst.pResolveAttachments = copyAttachmentReference(src.resolveAttachments);
                dst.pDepthStencilAttachment = copyAttachmentReference(src.depthStencilAttachments);
                dst.preserveAttachmentCount = static_cast<uint32_t>(src.preserveAttachments.size());
                dst.pPreserveAttachments = src.preserveAttachments.empty() ? nullptr : src.preserveAttachments.data();

                if (!src.depthStencilResolveAttachments.empty())
                {
                    auto depthStencilResolve = scratchMemory->allocate<VkSubpassDescriptionDepthStencilResolve>();
                    depthStencilResolve->sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
                    depthStencilResolve->pNext = nullptr;
                    depthStencilResolve->depthResolveMode = src.depthResolveMode;
                    depthStencilResolve->stencilResolveMode = src.stencilResolveMode;
                    depthStencilResolve->pDepthStencilResolveAttachment = copyAttachmentReference(src.depthStencilResolveAttachments);
                    ;

                    dst.pNext = depthStencilResolve;
                }
            }
            return vk_subpassDescription;
        };

        auto copySubpassDependency = [&scratchMemory](const Dependencies& subpassDependency) -> VkSubpassDependency2* {
            if (subpassDependency.empty()) return nullptr;

            auto vk_subpassDependencies = scratchMemory->allocate<VkSubpassDependency2>(subpassDependency.size());
            for (size_t i = 0; i < subpassDependency.size(); ++i)
            {
                auto& src = subpassDependency[i];
                auto& dst = vk_subpassDependencies[i];
                dst.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
                dst.pNext = nullptr;
                dst.srcSubpass = src.srcSubpass;
                dst.dstSubpass = src.dstSubpass;
                dst.srcStageMask = src.srcStageMask;
                dst.dstStageMask = src.dstStageMask;
                dst.srcAccessMask = src.srcAccessMask;
                dst.dstAccessMask = src.dstAccessMask;
                dst.dependencyFlags = src.dependencyFlags;
                dst.viewOffset = src.viewOffset;
            }
            return vk_subpassDependencies;
        };

        VkRenderPassCreateInfo2 renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = copyAttachmentDescriptions(attachments);
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
        renderPassInfo.pSubpasses = copySubpasses(subpasses);
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = copySubpassDependency(dependencies);
        renderPassInfo.correlatedViewMaskCount = static_cast<uint32_t>(correlatedViewMasks.size());
        renderPassInfo.pCorrelatedViewMasks = correlatedViewMasks.empty() ? nullptr : correlatedViewMasks.data();

        VkResult result = extensions->vkCreateRenderPass2(*device, &renderPassInfo, device->getAllocationCallbacks(), &_renderPass);
        if (result != VK_SUCCESS)
        {
            throw Exception{"Error: vsg::RenderPass::create(...) Failed to create VkRenderPass.", result};
        }
    }
    else
    {
        // Vulkan 1.0 vkCreateRenderPass code path
        auto copyAttachmentDescriptions = [&scratchMemory](const Attachments& attachmentDescriptions) -> VkAttachmentDescription* {
            if (attachmentDescriptions.empty()) return nullptr;

            auto vk_attachmentDescriptions = scratchMemory->allocate<VkAttachmentDescription>(attachmentDescriptions.size());
            for (size_t i = 0; i < attachmentDescriptions.size(); ++i)
            {
                auto& src = attachmentDescriptions[i];
                auto& dst = vk_attachmentDescriptions[i];

                dst.flags = src.flags;
                dst.format = src.format;
                dst.samples = src.samples;
                dst.loadOp = src.loadOp;
                dst.storeOp = src.storeOp;
                dst.stencilLoadOp = src.stencilLoadOp;
                dst.stencilStoreOp = src.stencilStoreOp;
                dst.initialLayout = src.initialLayout;
                dst.finalLayout = src.finalLayout;
            }
            return vk_attachmentDescriptions;
        };

        auto copySubpasses = [&scratchMemory](const Subpasses& subpassDescriptions) -> VkSubpassDescription* {
            if (subpassDescriptions.empty()) return nullptr;

            auto copyAttachmentReference = [&scratchMemory](const std::vector<AttachmentReference>& attachmentReferences) -> VkAttachmentReference* {
                if (attachmentReferences.empty()) return nullptr;

                auto vk_attachments = scratchMemory->allocate<VkAttachmentReference>(attachmentReferences.size());
                for (size_t i = 0; i < attachmentReferences.size(); ++i)
                {
                    auto& src = attachmentReferences[i];
                    auto& dst = vk_attachments[i];
                    dst.attachment = src.attachment;
                    dst.layout = src.layout;
                    // dst.aspectMask = src.aspectMask; // only VkAttachmentReference2
                }
                return vk_attachments;
            };

            auto vk_subpassDescription = scratchMemory->allocate<VkSubpassDescription>(subpassDescriptions.size());
            for (size_t i = 0; i < subpassDescriptions.size(); ++i)
            {
                auto& src = subpassDescriptions[i];
                if (!src.depthStencilResolveAttachments.empty())
                    vsg::warn("vsg::RenderPass::create(...): Subpass includes depthStencilResolveAttachments but vkCreateRenderPass2 is not enabled. Set WindowTraits::vulkanVersion to at least VK_API_VERSION_1_2 or add VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME to WindowTraits::deviceExtensionNames.");
                auto& dst = vk_subpassDescription[i];
                dst.flags = src.flags;
                dst.pipelineBindPoint = src.pipelineBindPoint;
                dst.inputAttachmentCount = static_cast<uint32_t>(src.inputAttachments.size());
                dst.pInputAttachments = copyAttachmentReference(src.inputAttachments);
                dst.colorAttachmentCount = static_cast<uint32_t>(src.colorAttachments.size());
                dst.pColorAttachments = copyAttachmentReference(src.colorAttachments);
                dst.pResolveAttachments = copyAttachmentReference(src.resolveAttachments);
                dst.pDepthStencilAttachment = copyAttachmentReference(src.depthStencilAttachments);
                dst.preserveAttachmentCount = static_cast<uint32_t>(src.preserveAttachments.size());
                dst.pPreserveAttachments = src.preserveAttachments.empty() ? nullptr : src.preserveAttachments.data();
            }
            return vk_subpassDescription;
        };

        auto copySubpassDependency = [&scratchMemory](const Dependencies& subpassDependency) -> VkSubpassDependency* {
            if (subpassDependency.empty()) return nullptr;

            auto vk_subpassDependencies = scratchMemory->allocate<VkSubpassDependency>(subpassDependency.size());
            for (size_t i = 0; i < subpassDependency.size(); ++i)
            {
                auto& src = subpassDependency[i];
                auto& dst = vk_subpassDependencies[i];
                dst.srcSubpass = src.srcSubpass;
                dst.dstSubpass = src.dstSubpass;
                dst.srcStageMask = src.srcStageMask;
                dst.dstStageMask = src.dstStageMask;
                dst.srcAccessMask = src.srcAccessMask;
                dst.dstAccessMask = src.dstAccessMask;
                dst.dependencyFlags = src.dependencyFlags;
                //dst.viewOffset = src.viewOffset; // only VkSubpassDependency2
            }
            return vk_subpassDependencies;
        };

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = copyAttachmentDescriptions(attachments);
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
        renderPassInfo.pSubpasses = copySubpasses(subpasses);
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = copySubpassDependency(dependencies);

        VkResult result = vkCreateRenderPass(*device, &renderPassInfo, device->getAllocationCallbacks(), &_renderPass);
        if (result != VK_SUCCESS)
        {
            throw Exception{"Error: vsg::RenderPass::create(...) Failed to create VkRenderPass.", result};
        }
    }
}

RenderPass::~RenderPass()
{
    if (_renderPass)
    {
        vkDestroyRenderPass(*device, _renderPass, device->getAllocationCallbacks());
    }
}

AttachmentDescription vsg::defaultColorAttachment(VkFormat imageFormat)
{
    AttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    return colorAttachment;
}

AttachmentDescription vsg::defaultDepthAttachment(VkFormat depthFormat)
{
    AttachmentDescription depthAttachment = {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return depthAttachment;
}

ref_ptr<RenderPass> vsg::createRenderPass(Device* device, VkFormat imageFormat, VkFormat depthFormat, bool requiresDepthRead)
{
    auto colorAttachment = defaultColorAttachment(imageFormat);
    auto depthAttachment = defaultDepthAttachment(depthFormat);

    if (requiresDepthRead)
    {
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }

    RenderPass::Attachments attachments{colorAttachment, depthAttachment};

    AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    AttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    SubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachments.emplace_back(colorAttachmentRef);
    subpass.depthStencilAttachments.emplace_back(depthAttachmentRef);

    RenderPass::Subpasses subpasses{subpass};

    // image layout transition
    SubpassDependency colorDependency = {};
    colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    colorDependency.dstSubpass = 0;
    colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    colorDependency.srcAccessMask = 0;
    colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    colorDependency.dependencyFlags = 0;

    // depth buffer is shared between swap chain images
    SubpassDependency depthDependency = {};
    depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    depthDependency.dstSubpass = 0;
    depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthDependency.dependencyFlags = 0;

    RenderPass::Dependencies dependencies{colorDependency, depthDependency};

    return RenderPass::create(device, attachments, subpasses, dependencies);
}

ref_ptr<RenderPass> vsg::createMultisampledRenderPass(Device* device, VkFormat imageFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, bool requiresDepthRead)
{
    if (samples == VK_SAMPLE_COUNT_1_BIT)
    {
        return createRenderPass(device, imageFormat, depthFormat, requiresDepthRead);
    }

    // First attachment is multisampled target.
    AttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = samples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Second attachment is the resolved image which will be presented.
    AttachmentDescription resolveAttachment = {};
    resolveAttachment.format = imageFormat;
    resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Multisampled depth attachment. It won't be resolved.
    AttachmentDescription depthAttachment = {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = samples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RenderPass::Attachments attachments{colorAttachment, resolveAttachment, depthAttachment};

    if (requiresDepthRead)
    {
        AttachmentDescription depthResolveAttachment = {};
        depthResolveAttachment.format = depthFormat;
        depthResolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthResolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthResolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthResolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthResolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthResolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthResolveAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(depthResolveAttachment);
    }

    AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    AttachmentReference resolveAttachmentRef = {};
    resolveAttachmentRef.attachment = 1;
    resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    AttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    SubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachments.emplace_back(colorAttachmentRef);
    subpass.resolveAttachments.emplace_back(resolveAttachmentRef);
    subpass.depthStencilAttachments.emplace_back(depthAttachmentRef);

    if (requiresDepthRead)
    {
        AttachmentReference depthResolveAttachmentRef = {};
        depthResolveAttachmentRef.attachment = 3;
        depthResolveAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        subpass.stencilResolveMode = VK_RESOLVE_MODE_NONE;
        subpass.depthStencilResolveAttachments.emplace_back(depthResolveAttachmentRef);
    }

    RenderPass::Subpasses subpasses{subpass};

    SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    SubpassDependency dependency2 = {};
    dependency2.srcSubpass = 0;
    dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency2.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    RenderPass::Dependencies dependencies{dependency, dependency2};

    return RenderPass::create(device, attachments, subpasses, dependencies);
}

ref_ptr<RenderPass> vsg::createRenderPass(Device* device, VkFormat imageFormat)
{
    auto colorAttachment = defaultColorAttachment(imageFormat);

    RenderPass::Attachments attachments{colorAttachment};

    AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    SubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachments.emplace_back(colorAttachmentRef);

    RenderPass::Subpasses subpasses{subpass};

    // image layout transition
    SubpassDependency colorDependency = {};
    colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    colorDependency.dstSubpass = 0;
    colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    colorDependency.srcAccessMask = 0;
    colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    colorDependency.dependencyFlags = 0;

    RenderPass::Dependencies dependencies{colorDependency};

    return RenderPass::create(device, attachments, subpasses, dependencies);
}

ref_ptr<RenderPass> vsg::createMultisampledRenderPass(Device* device, VkFormat imageFormat, VkSampleCountFlagBits samples)
{
    if (samples == VK_SAMPLE_COUNT_1_BIT)
    {
        return createRenderPass(device, imageFormat);
    }

    // First attachment is multisampled target.
    AttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = samples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Second attachment is the resolved image which will be presented.
    AttachmentDescription resolveAttachment = {};
    resolveAttachment.format = imageFormat;
    resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RenderPass::Attachments attachments{colorAttachment, resolveAttachment};

    AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    AttachmentReference resolveAttachmentRef = {};
    resolveAttachmentRef.attachment = 1;
    resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    SubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachments.emplace_back(colorAttachmentRef);
    subpass.resolveAttachments.emplace_back(resolveAttachmentRef);

    RenderPass::Subpasses subpasses{subpass};

    SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    SubpassDependency dependency2 = {};
    dependency2.srcSubpass = 0;
    dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency2.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    RenderPass::Dependencies dependencies{dependency, dependency2};

    return RenderPass::create(device, attachments, subpasses, dependencies);
}
