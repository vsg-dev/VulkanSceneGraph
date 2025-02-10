/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/io/write.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/HardShadows.h>
#include <vsg/lighting/PercentageCloserSoftShadows.h>
#include <vsg/lighting/PointLight.h>
#include <vsg/lighting/SoftShadows.h>
#include <vsg/lighting/SpotLight.h>
#include <vsg/nodes/RegionOfInterest.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/Context.h>

using namespace vsg;

//////////////////////////////////////
//
// TraverseChildrenOfNode
//
namespace vsg
{
    class TraverseChildrenOfNode : public Inherit<Node, TraverseChildrenOfNode>
    {
    public:
        explicit TraverseChildrenOfNode(Node* in_node) :
            node(in_node) {}

        observer_ptr<Node> node;

        template<class N, class V>
        static void t_traverse(N& in_node, V& visitor)
        {
            if (auto ref_node = in_node.node.ref_ptr()) ref_node->traverse(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }
    };
    VSG_type_name(TraverseChildrenOfNode);

    inline double Cpractical(double n, double f, double i, double m, double lambda)
    {
        double Clog = n * std::pow((f / n), (i / m));
        double Cuniform = n + (f - n) * (i / m);
        return Clog * lambda + Cuniform * (1.0 - lambda);
    };

} // namespace vsg

//////////////////////////////////////
//
// ViewDescriptorSetLayout
//
ViewDescriptorSetLayout::ViewDescriptorSetLayout()
{
}

int ViewDescriptorSetLayout::compare(const Object& rhs_object) const
{
    int result = DescriptorSetLayout::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_pointer(_viewDescriptorSetLayout, rhs._viewDescriptorSetLayout);
}

void ViewDescriptorSetLayout::read(Input& input)
{
    Object::read(input);
}

void ViewDescriptorSetLayout::write(Output& output) const
{
    Object::write(output);
}

void ViewDescriptorSetLayout::compile(Context& context)
{
    if (!_viewDescriptorSetLayout && context.viewDependentState && context.viewDependentState->descriptorSetLayout)
    {
        _viewDescriptorSetLayout = context.viewDependentState->descriptorSetLayout;
        _viewDescriptorSetLayout->compile(context);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BindViewDescriptorSets
//
BindViewDescriptorSets::BindViewDescriptorSets() :
    Inherit(2), // slot 2
    pipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
    firstSet(0)
{
}

int BindViewDescriptorSets::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(pipelineBindPoint, rhs.pipelineBindPoint))) return result;
    if ((result = compare_pointer(layout, rhs.layout))) return result;
    return compare_value(firstSet, rhs.firstSet);
}

void BindViewDescriptorSets::read(Input& input)
{
    StateCommand::read(input);

    input.readValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    input.read("layout", layout);
    input.read("firstSet", firstSet);
}

void BindViewDescriptorSets::write(Output& output) const
{
    StateCommand::write(output);

    output.writeValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    output.write("layout", layout);
    output.write("firstSet", firstSet);
}

void BindViewDescriptorSets::compile(Context& context)
{
    layout->compile(context);
    if (context.viewDependentState) context.viewDependentState->compile(context);
}

void BindViewDescriptorSets::record(CommandBuffer& commandBuffer) const
{
    if (commandBuffer.viewDependentState)
    {
        commandBuffer.viewDependentState->bindDescriptorSets(commandBuffer, pipelineBindPoint, layout->vk(commandBuffer.deviceID), firstSet);
    }
}

//////////////////////////////////////
//
// ViewDependentState
//
ViewDependentState::ViewDependentState(View* in_view) :
    view(in_view)
{
    // info("ViewDependentState::ViewDependentState(view = ", view, ")");
}

ViewDependentState::~ViewDependentState()
{
}

ref_ptr<Image> createShadowImage(uint32_t width, uint32_t height, uint32_t levels, VkFormat format, VkImageUsageFlags usage)
{
    auto image = Image::create();
    image->imageType = VK_IMAGE_TYPE_2D;
    image->format = format;
    image->extent = VkExtent3D{width, height, 1};
    image->mipLevels = 1;
    image->arrayLayers = levels;
    image->samples = VK_SAMPLE_COUNT_1_BIT;
    image->tiling = VK_IMAGE_TILING_OPTIMAL;
    image->usage = usage;
    image->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image->flags = 0;
    image->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return image;
}

ref_ptr<ShadowSettings> ViewDependentState::getActiveShadowSettings(const Light* light) const
{
    // find an exact match
    auto itr = shadowSettingsOverride.find(ref_ptr<const Light>(light));
    if (itr != shadowSettingsOverride.end())
    {
        return itr->second;
    }

    // if null entry exists then use it to override all unmatched Lights.
    itr = shadowSettingsOverride.find({});
    if (itr != shadowSettingsOverride.end())
    {
        return itr->second;
    }

    return light->shadowSettings;
}

void ViewDependentState::init(ResourceRequirements& requirements)
{
    // check if ViewDependentState has already been initialized
    if (lightData) return;

    if (!shaderSet)
    {
        // fallback to using the standard PBR ShaderSet
        shaderSet = vsg::createPhysicsBasedRenderingShaderSet();
    }

    auto descriptorConfigurator = DescriptorConfigurator::create(shaderSet);

    uint32_t maxNumberLights = 64;
    uint32_t maxViewports = 1;

    uint32_t shadowWidth = 2048;
    uint32_t shadowHeight = 2048;
    uint32_t maxShadowMaps = 8;

    const auto& viewDetails = requirements.views[view];

    if ((view->features & (RECORD_LIGHTS | RECORD_SHADOW_MAPS)) != 0)
    {
        uint32_t numLights = static_cast<uint32_t>(viewDetails.lights.size());
        uint32_t numShadowMaps = 0;
        for (auto& light : viewDetails.lights)
        {
            if (auto shadowSettings = getActiveShadowSettings(light))
            {
                numShadowMaps += shadowSettings->shadowMapCount;
            }
        }

        if (numLights < requirements.numLightsRange[0])
            maxNumberLights = requirements.numLightsRange[0];
        else if (numLights > requirements.numLightsRange[1])
            maxNumberLights = requirements.numLightsRange[1];
        else
            maxNumberLights = numLights;

        if (numShadowMaps < requirements.numShadowMapsRange[0])
            maxShadowMaps = requirements.numShadowMapsRange[0];
        else if (numShadowMaps > requirements.numShadowMapsRange[1])
            maxShadowMaps = requirements.numShadowMapsRange[1];
        else
            maxShadowMaps = numShadowMaps;

        shadowWidth = requirements.shadowMapSize.x;
        shadowHeight = requirements.shadowMapSize.y;
    }
    else
    {
        maxNumberLights = 0;
        maxShadowMaps = 0;
    }

    // 1 vec3 is used for specifying the number of lights, lagest lightData entries are for spot light with 4 vec4s per light, and each shadowmap takes 8 vec4s.
    uint32_t lightDataSize = 1 + maxNumberLights * 4 + maxShadowMaps * 8;

#if 0
    if (active)
    {
        info("void ViewDependentState::init(ResourceRequirements& requirements) view = ", view, ", active = ", active);
        info("    viewDetails.indices.size() = ", viewDetails.indices.size());
        info("    viewDetails.bins.size() = ", viewDetails.bins.size());
        info("    viewDetails.lights.size() = ", viewDetails.lights.size());
        info("    maxViewports = ", maxViewports);
        info("    maxNumberLights = ", maxNumberLights);
        info("    maxShadowMaps = ", maxShadowMaps);
        info("    lightDataSize = ", lightDataSize);
        info("    shadowWidth = ", shadowWidth);
        info("    shadowHeight = ", shadowHeight);
        info("    requirements.numLightsRange = ", requirements.numLightsRange);
        info("    requirements.numShadowMapsRange = ", requirements.numShadowMapsRange);
        info("    requirements.shadowMapSize = ", requirements.shadowMapSize);
    }
#endif

    lightData = vec4Array::create(lightDataSize);
    lightData->setValue("name", "lightData");
    lightData->properties.dataVariance = DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
    lightDataBufferInfo = BufferInfo::create(lightData.get());
    descriptorConfigurator->assignDescriptor("lightData", BufferInfoList{lightDataBufferInfo});

    viewportData = vec4Array::create(maxViewports);
    viewportData->setValue("name", "viewportData");
    viewportData->properties.dataVariance = DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
    viewportDataBufferInfo = BufferInfo::create(viewportData.get());
    descriptorConfigurator->assignDescriptor("viewportData", BufferInfoList{viewportDataBufferInfo});

    // set up ShadowMaps
    auto shadowMapDirectSampler = Sampler::create();
    shadowMapDirectSampler->minFilter = VK_FILTER_NEAREST;
    shadowMapDirectSampler->magFilter = VK_FILTER_NEAREST;
    shadowMapDirectSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    shadowMapDirectSampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadowMapDirectSampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadowMapDirectSampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    auto shadowMapSampler = Sampler::create();
    shadowMapSampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadowMapSampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadowMapSampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
#define HARDWARE_PCF 1
#if HARDWARE_PCF == 1
    shadowMapSampler->minFilter = VK_FILTER_LINEAR;
    shadowMapSampler->magFilter = VK_FILTER_LINEAR;
    shadowMapSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    shadowMapSampler->compareEnable = VK_TRUE;
    shadowMapSampler->compareOp = VK_COMPARE_OP_LESS;
#else
    shadowMapSampler->minFilter = VK_FILTER_NEAREST;
    shadowMapSampler->magFilter = VK_FILTER_NEAREST;
    shadowMapSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
#endif
    if (maxShadowMaps > 0)
    {
        shadowDepthImage = createShadowImage(shadowWidth, shadowHeight, maxShadowMaps, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        auto depthImageView = ImageView::create(shadowDepthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
        depthImageView->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        depthImageView->subresourceRange.baseMipLevel = 0;
        depthImageView->subresourceRange.levelCount = 1;
        depthImageView->subresourceRange.baseArrayLayer = 0;
        depthImageView->subresourceRange.layerCount = maxShadowMaps;

        auto depthImageInfo = ImageInfo::create(nullptr, depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

        descriptorConfigurator->assignTexture("shadowMaps", ImageInfoList{depthImageInfo});
    }
    else
    {
        //
        // fallback to provide a descriptor image to use when the ViewDependentState shadow map generation is not active
        //
        Data::Properties properties;
        properties.format = VK_FORMAT_D32_SFLOAT;
        properties.imageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

        auto shadowMapData = floatArray3D::create(1, 1, 1, 0.0f, properties);
        shadowDepthImage = Image::create(shadowMapData);
        shadowDepthImage->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        auto depthImageView = ImageView::create(shadowDepthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
        depthImageView->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        depthImageView->subresourceRange.baseMipLevel = 0;
        depthImageView->subresourceRange.levelCount = 1;
        depthImageView->subresourceRange.baseArrayLayer = 0;
        depthImageView->subresourceRange.layerCount = 1;

        auto depthImageInfo = ImageInfo::create(nullptr, depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

        descriptorConfigurator->assignTexture("shadowMaps", ImageInfoList{depthImageInfo});
    }

    auto shadowMapDirectSamplerInfo = ImageInfo::create(shadowMapDirectSampler, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    auto shadowMapSamplerInfo = ImageInfo::create(shadowMapSampler, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    descriptorConfigurator->assignTexture("shadowMapDirectSampler", ImageInfoList{shadowMapDirectSamplerInfo});
    descriptorConfigurator->assignTexture("shadowMapShadowSampler", ImageInfoList{shadowMapSamplerInfo});

    // assign the DescriptorSet and layout created by the descriptorConfigurator
    for (size_t set = 0; set < descriptorConfigurator->descriptorSets.size(); ++set)
    {
        if (auto ds = descriptorConfigurator->descriptorSets[set])
        {
            descriptorSet = ds;
            descriptorSetLayout = ds->setLayout;
        }
    }

    // if not active then don't enable shadow maps
    if (maxShadowMaps == 0) return;

    // create a switch to toggle on/off the render to texture subgraphs for each shadowmap layer
    preRenderSwitch = Switch::create();

    preRenderCommandGraph = CommandGraph::create();
    preRenderCommandGraph->submitOrder = -1;
    preRenderCommandGraph->addChild(preRenderSwitch);

    auto tcon = TraverseChildrenOfNode::create(view);

    Mask shadowMask = 0x1; // TODO: do we inherit from main scene? how?

    auto viewportState = ViewportState::create(VkExtent2D{shadowWidth, shadowHeight});

    ref_ptr<View> first_view;
    shadowMaps.resize(maxShadowMaps);
    for (auto& shadowMap : shadowMaps)
    {
        if (first_view)
        {
            shadowMap.view = View::create(*first_view);
        }
        else
        {
            first_view = View::create(ViewFeatures(INHERIT_VIEWPOINT));
            shadowMap.view = first_view;
        }

        shadowMap.view->mask = shadowMask;
        shadowMap.view->camera = Camera::create();
        shadowMap.view->addChild(tcon);
        shadowMap.view->camera->viewportState = viewportState;

        shadowMap.renderGraph = RenderGraph::create();
        shadowMap.renderGraph->addChild(shadowMap.view);
        preRenderSwitch->addChild(MASK_ALL, shadowMap.renderGraph);
    }
}

void ViewDependentState::update(ResourceRequirements& requirements)
{
    if (preRenderCommandGraph && requirements.maxSlot > preRenderCommandGraph->maxSlot)
    {
        preRenderCommandGraph->maxSlot = requirements.maxSlot;
    }
}

void ViewDependentState::compile(Context& context)
{
    if (compiled) return;
    compiled = true;

    CPU_INSTRUMENTATION_L1_NC(context.instrumentation, "ViewDependentState compile", COLOR_COMPILE);

    descriptorSet->compile(context);

    if ((view->features & RECORD_SHADOW_MAPS) != 0 && preRenderCommandGraph && !preRenderCommandGraph->device)
    {
        preRenderCommandGraph->device = context.device;

        // TODO
        preRenderCommandGraph->queueFamily = 0;

        auto extent = shadowDepthImage->extent;

        shadowDepthImage->compile(context);

        uint32_t layer = 0;
        for (const auto& shadowMap : shadowMaps)
        {
            // create depth buffer
            auto depthImageView = ImageView::create(shadowDepthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
            depthImageView->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            depthImageView->subresourceRange.baseMipLevel = 0;
            depthImageView->subresourceRange.levelCount = 1;
            depthImageView->subresourceRange.baseArrayLayer = layer;
            depthImageView->subresourceRange.layerCount = 1;
            depthImageView->compile(context);

            // attachment descriptions
            RenderPass::Attachments attachments(1);
            // Depth attachment
            attachments[0].format = shadowDepthImage->format;
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            AttachmentReference ignoreColorReference = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};
            AttachmentReference depthReference = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
            RenderPass::Subpasses subpassDescription(1);
            subpassDescription[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription[0].colorAttachments.emplace_back(ignoreColorReference);
            subpassDescription[0].depthStencilAttachments.emplace_back(depthReference);

            RenderPass::Dependencies dependencies(2);

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            auto renderPass = RenderPass::create(context.device, attachments, subpassDescription, dependencies);

            // Framebuffer
            auto fbuf = Framebuffer::create(renderPass, ImageViews{depthImageView}, extent.width, extent.height, 1);

            auto rendergraph = shadowMap.renderGraph;
            rendergraph->renderArea.offset = VkOffset2D{0, 0};
            rendergraph->renderArea.extent = VkExtent2D{extent.width, extent.height};
            rendergraph->framebuffer = fbuf;

            rendergraph->clearValues.resize(1);
            rendergraph->clearValues[0].depthStencil = VkClearDepthStencilValue{0.0f, 0};

            ++layer;
        }

        // use an image barrier to transition the initial shadow map array layout to VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        // so that the whole shadow map is usable in fragment shader even when only portions of it have been set using a render to texture pass
        auto initLayoutBarrier = ImageMemoryBarrier::create(
            0,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            shadowDepthImage,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, static_cast<uint32_t>(shadowMaps.size())});

        auto pipelinBarrier = PipelineBarrier::create(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, initLayoutBarrier);

        context.commands.push_back(pipelinBarrier);
    }
}

void ViewDependentState::clear()
{
    //debug("ViewDependentState::clear() bufferIndex = ", bufferIndex);

    // clear data
    ambientLights.clear();
    directionalLights.clear();
    pointLights.clear();
    spotLights.clear();
}

void ViewDependentState::traverse(RecordTraversal& rt) const
{
    //GPU_INSTRUMENTATION_L1_NC(rt.instrumentation, *rt.getCommandBuffer(), "ViewDependentState", COLOR_RECORD_L1);
    CPU_INSTRUMENTATION_L1_NC(rt.instrumentation, "ViewDependentState", COLOR_RECORD_L1);

    if ((view->features & RECORD_SHADOW_MAPS) == 0) return;

    // useful reference : https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps
    // PCF filtering : https://github.com/SaschaWillems/Vulkan/issues/231
    // sampler2DArrayShadow
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineDepthStencilStateCreateInfo.html
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCmdSetDepthBoundsTestEnable.html
    //
    // Game industry SIGGRAPH presentation
    // https://www.realtimeshadows.com/sites/default/files/Playing%20with%20Real-Time%20Shadows_0.pdf
    //
    // Soft shadows:
    // https://ogldev.org/www/tutorial42/tutorial42.html
    // https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
    // https://andrew-pham.blog/2019/08/03/percentage-closer-soft-shadows/
    // https://github.com/vsgopenmw-dev/vsgopenmw/blob/master/files/shaders/lib/view/shadow.glsl

    bool requiresPerRenderShadowMaps = false;
    uint32_t shadowMapIndex = 0;
    uint32_t numShadowMaps = static_cast<uint32_t>(shadowMaps.size());
    if (preRenderSwitch)
        preRenderSwitch->setAllChildren(false);
    else
        numShadowMaps = 0;

    auto computeFrustumBounds = [&](double n, double f, const dmat4& clipToWorld) -> dbox {
        dbox bounds;
        bounds.add(clipToWorld * dvec3(-1.0, -1.0, n));
        bounds.add(clipToWorld * dvec3(-1.0, 1.0, n));
        bounds.add(clipToWorld * dvec3(1.0, -1.0, n));
        bounds.add(clipToWorld * dvec3(1.0, 1.0, n));
        bounds.add(clipToWorld * dvec3(-1.0, -1.0, f));
        bounds.add(clipToWorld * dvec3(-1.0, 1.0, f));
        bounds.add(clipToWorld * dvec3(1.0, -1.0, f));
        bounds.add(clipToWorld * dvec3(1.0, 1.0, f));

        return bounds;
    };

    // clip against near plane
    // Converting between homogeneous coordinates and Cartesian coordinates can turn internal line segements
    // (the section of the line between two points) into external line segments (the line except the part
    // between the points). In particular, this happens for ones that cross the near plane of a perspective
    // projection. This function therefore excludes the section of the frustum on the wrong side of the near
    // plane, sidestepping the problem, and avoiding giving infinite bounds for infinite external line segments.
    auto computeFrustumBoundsClipped = [&](double n, double f, const dmat4& clipToWorld) -> dbox {
        std::array<dvec4, 8> corners{{
            clipToWorld * dvec4(-1.0, -1.0, n, 1.0),
            clipToWorld * dvec4(-1.0, 1.0, n, 1.0),
            clipToWorld * dvec4(1.0, -1.0, n, 1.0),
            clipToWorld * dvec4(1.0, 1.0, n, 1.0),
            clipToWorld * dvec4(-1.0, -1.0, f, 1.0),
            clipToWorld * dvec4(-1.0, 1.0, f, 1.0),
            clipToWorld * dvec4(1.0, -1.0, f, 1.0),
            clipToWorld * dvec4(1.0, 1.0, f, 1.0),
        }};
        std::array<std::pair<int, int>, 12> edges{{
            {0, 1},
            {1, 3},
            {3, 2},
            {2, 0},
            {0, 4},
            {1, 5},
            {2, 6},
            {3, 7},
            {4, 5},
            {5, 7},
            {7, 6},
            {6, 4},
        }};

        dbox bounds;

        std::array<bool, 8> clipped;
        for (unsigned int i = 0; i < corners.size(); ++i)
        {
            clipped[i] = corners[i].w < corners[i].z;
            if (!clipped[i])
                bounds.add(corners[i].xyz / corners[i].w);
        }
        for (const auto& [start, end] : edges)
        {
            if (clipped[start] != clipped[end])
            {
                // add the point where z=w on the line
                const auto& p1 = corners[start];
                const auto& p2 = corners[end];
                double a = (p1.z - p1.w) / (p1.z - p2.z - p1.w + p2.w);
                dvec4 p = p1 * (1.0 - a) + p2 * a;
                bounds.add(p.xyz / p.w);
            }
        }
        return bounds;
    };

    // info("\n\nViewDependentState::traverse(", &rt, ", ", &view, ") numShadowMaps = ", numShadowMaps);

    // cache general view parameters
    auto projectionMatrix = view->camera->projectionMatrix->transform();
    auto viewMatrix = view->camera->viewMatrix->transform();
    auto inverse_viewMatrix = inverse(viewMatrix);
    auto view_direction = normalize(dvec3(0.0, 0.0, -1.0) * (projectionMatrix * viewMatrix));
    auto view_up = normalize(dvec3(0.0, -1.0, 0.0) * (projectionMatrix * viewMatrix));

    auto clipToEye = inverse(projectionMatrix);
    auto n = -(clipToEye * dvec3(0.0, 0.0, 1.0)).z;
    auto f = -(clipToEye * dvec3(0.0, 0.0, 0.0)).z;

    // if regions of interest have been found in the scene graph use them to clamp the near/far values.
    if (!rt.regionsOfInterest.empty())
    {
        dbox eyeSpaceRegionBounds;
        for (auto& [mv, regionOfInterest] : rt.regionsOfInterest)
        {
            for (auto& v : regionOfInterest->points)
            {
                eyeSpaceRegionBounds.add(mv * v);
            }
        }

        if (eyeSpaceRegionBounds)
        {
            double regionNear = -eyeSpaceRegionBounds.max.z;
            double regionFar = -eyeSpaceRegionBounds.min.z;
            if (regionNear > n) { n = regionNear; }
            if (regionFar < f) { f = regionFar; }
        }
    }

    // set up the light data
    auto light_itr = lightData->begin();
    lightData->dirty();

    (*light_itr++) = vec4(static_cast<float>(ambientLights.size()),
                          static_cast<float>(directionalLights.size()),
                          static_cast<float>(pointLights.size()),
                          static_cast<float>(spotLights.size()));

    // lightData requirements = vec4 * (num_ambientLights + 3 * num_directionLights + 3 * num_pointLights + 4 * num_spotLights + 4 * num_shadow_maps)

    for (const auto& entry : ambientLights)
    {
        auto light = entry.second;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
    }

    for (auto& [mv, light] : directionalLights)
    {
        // info("   light ", light->className(), ", light->shadowMapCount = ", light->shadowMapCount);

        // assign basic direction light settings to light data
        auto eye_direction = normalize(light->direction * inverse_3x3(mv));
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_direction.x), static_cast<float>(eye_direction.y), static_cast<float>(eye_direction.z), 0.0f);

        auto shadowSettings = getActiveShadowSettings(light);
        uint32_t activeNumShadowMaps = shadowSettings ? std::min(shadowSettings->shadowMapCount, numShadowMaps - shadowMapIndex) : 0;
        if (shadowSettings)
        {
            if (shadowSettings->type_info() == typeid(HardShadows))
            {
                (*light_itr++).set(static_cast<float>(activeNumShadowMaps), -1.0f, -1.0f, 0.0f);
            }
            else if (shadowSettings->type_info() == typeid(SoftShadows))
            {
                const SoftShadows& pcfShadowSettings = static_cast<const SoftShadows&>(*shadowSettings);
                (*light_itr++).set(static_cast<float>(activeNumShadowMaps), pcfShadowSettings.penumbraRadius, -1.0f, 0.0f);
            }
            else if (shadowSettings->type_info() == typeid(PercentageCloserSoftShadows))
            {
                (*light_itr++).set(static_cast<float>(activeNumShadowMaps), 0.1f /* todo: calculate blocker search radius */, std::tan(light->angleSubtended / 2), 0.0f);
            }
        }
        else
            (*light_itr++).set(0.0f, 0.0f, 0.0f, 0.0f);

        if (activeNumShadowMaps == 0) continue;

        // set up shadow map rendering backend
        requiresPerRenderShadowMaps = true;

        // compute directional light space
        // light direction in world coords
        auto light_direction = normalize(light->direction * (inverse_3x3(mv * inverse_viewMatrix)));
#if 0
        info("   directional light : light direction in world = ", light_direction, ", light->shadowMapCount = ", light->shadowMapCount);
        info("      light->direction in model = ", light->direction);
        info("      view_direction in world = ", view_direction);
        info("      view_up in world = ", view_up);
#endif
        auto light_x_direction = cross(light_direction, view_direction);
        auto light_x_up = cross(light_direction, view_up);

        auto light_x = (length(light_x_direction) > length(light_x_up)) ? normalize(light_x_direction) : normalize(light_x_up);
        auto light_y = cross(light_x, light_direction);
        auto light_z = light_direction;

        // clamp the near and far values
        if (n > maxShadowDistance)
        {
            // near plane further than maximum shadow distance so no need to generate shadow maps
            continue;
        }
        if (f > maxShadowDistance)
        {
            f = maxShadowDistance;
        }

        auto updateCamera = [&](double clip_near_z, double clip_far_z, const dmat4& clipToWorld) -> void {
            const auto& shadowMap = shadowMaps[shadowMapIndex];
            preRenderSwitch->children[shadowMapIndex].mask = MASK_ALL;

            const auto& camera = shadowMap.view->camera;
            auto lookAt = camera->viewMatrix.cast<LookAt>();
            auto ortho = camera->projectionMatrix.cast<Orthographic>();

            if (!lookAt) camera->viewMatrix = lookAt = LookAt::create();
            if (!ortho) camera->projectionMatrix = ortho = Orthographic::create();

            auto ws_bounds = computeFrustumBounds(clip_near_z, clip_far_z, clipToWorld);
            auto sm_eye = (ws_bounds.min + ws_bounds.max) * 0.5 - light_z * (0.5 * length(ws_bounds.max - ws_bounds.min));

            lookAt->eye = sm_eye;
            lookAt->center = sm_eye + light_z;
            lookAt->up = light_y;

            auto ls_bounds = computeFrustumBounds(clip_near_z, clip_far_z, lookAt->transform() * clipToWorld);

            ortho->left = ls_bounds.min.x;
            ortho->right = ls_bounds.max.x;
            ortho->bottom = ls_bounds.min.y;
            ortho->top = ls_bounds.max.y;
            ortho->nearDistance = -ls_bounds.max.z;
            ortho->farDistance = -ls_bounds.min.z;

            dmat4 shadowMapProjView = camera->projectionMatrix->transform() * camera->viewMatrix->transform();

            dmat4 shadowMapTM = scale(0.5, 0.5, 1.0) * translate(1.0, 1.0, shadowMapBias) * shadowMapProjView * inverse_viewMatrix;

            // convert tex gen matrix to float matrix and assign to light data
            mat4 m(shadowMapTM);

            (*light_itr++) = m[0];
            (*light_itr++) = m[1];
            (*light_itr++) = m[2];
            (*light_itr++) = m[3];

            // info("m = ", m);

            m = inverse(m);

            (*light_itr++) = m[0];
            (*light_itr++) = m[1];
            (*light_itr++) = m[2];
            (*light_itr++) = m[3];

            // advance to the next shadowMap
            shadowMapIndex++;
        };

#if 0
        info("     light_x = ", light_x);
        info("     light_y = ", light_y);
        info("     light_z = ", light_z);
#endif

#if 0
        double range = f - n;
        info("    n = ", n, ", f = ", f, ", range = ", range);
#endif
        auto clipToWorld = inverse(projectionMatrix * viewMatrix);

        if (activeNumShadowMaps > 1)
        {
            double m = static_cast<double>(activeNumShadowMaps);
            for (double i = 0; i < m; i += 1.0)
            {
                dvec3 eye_near(0.0, 0.0, -Cpractical(n, f, i, m, lambda));
                dvec3 eye_far(0.0, 0.0, -Cpractical(n, f, i + 1.0, m, lambda));

                auto clip_near = projectionMatrix * eye_near;
                auto clip_far = projectionMatrix * eye_far;

                updateCamera(clip_near.z, clip_far.z, clipToWorld);
            }
        }
        else
        {
            dvec3 eye_near(0.0, 0.0, -n);
            dvec3 eye_far(0.0, 0.0, -f);

            auto clip_near = projectionMatrix * eye_near;
            auto clip_far = projectionMatrix * eye_far;

            updateCamera(clip_near.z, clip_far.z, clipToWorld);
        }
    }

    for (auto& [mv, light] : pointLights)
    {
        auto eye_position = mv * light->position;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_position.x), static_cast<float>(eye_position.y), static_cast<float>(eye_position.z), 0.0f);
    }

    for (auto& [mv, light] : spotLights)
    {
        auto eye_position = mv * light->position;
        auto eye_direction = normalize(light->direction * inverse_3x3(mv));
        float cos_innerAngle = static_cast<float>(cos(light->innerAngle));
        float cos_outerAngle = static_cast<float>(cos(light->outerAngle));
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_position.x), static_cast<float>(eye_position.y), static_cast<float>(eye_position.z), cos_innerAngle);
        (*light_itr++).set(static_cast<float>(eye_direction.x), static_cast<float>(eye_direction.y), static_cast<float>(eye_direction.z), cos_outerAngle);

        auto shadowSettings = getActiveShadowSettings(light);
        uint32_t activeNumShadowMaps = shadowSettings ? std::min(shadowSettings->shadowMapCount, numShadowMaps - shadowMapIndex) : 0;
        if (shadowSettings)
        {
            if (shadowSettings->type_info() == typeid(HardShadows))
            {
                (*light_itr++).set(static_cast<float>(activeNumShadowMaps), -1.0f, -1.0f, 0.0f);
            }
            else if (shadowSettings->type_info() == typeid(SoftShadows))
            {
                const SoftShadows& pcfShadowSettings = static_cast<const SoftShadows&>(*shadowSettings);
                (*light_itr++).set(static_cast<float>(activeNumShadowMaps), pcfShadowSettings.penumbraRadius, -1.0f, 0.0f);
            }
            else if (shadowSettings->type_info() == typeid(PercentageCloserSoftShadows))
            {
                (*light_itr++).set(static_cast<float>(activeNumShadowMaps), 0.1f /* todo: calculate blocker search radius */, static_cast<float>(light->radius), 0.0f);
            }
        }
        else
            (*light_itr++).set(0.0f, 0.0f, 0.0f, 0.0f);

        if (activeNumShadowMaps == 0) continue;

        // set up shadow map rendering backend
        requiresPerRenderShadowMaps = true;

        // compute spot light space
        // light direction in world coords
        auto light_direction = normalize(light->direction * (inverse_3x3(mv * inverse_viewMatrix)));
        auto light_position = inverse_viewMatrix * mv * light->position;
#if 0
        info("   spot light : light direction in world = ", light_direction, ", light->shadowMapCount = ", light->shadowMapCount);
        info("      light->direction in model = ", light->direction);
        info("      view_direction in world = ", view_direction);
        info("      view_up in world = ", view_up);
#endif
        auto light_x_direction = cross(light_direction, view_direction);
        auto light_x_up = cross(light_direction, view_up);

        auto light_x = (length(light_x_direction) > length(light_x_up)) ? normalize(light_x_direction) : normalize(light_x_up);
        auto light_y = cross(light_x, light_direction);
        auto light_z = light_direction;

        // clamp the near and far values
        if (n > maxShadowDistance)
        {
            // near plane further than maximum shadow distance so no need to generate shadow maps
            continue;
        }
        if (f > maxShadowDistance)
        {
            f = maxShadowDistance;
        }

        auto light_outerAngle = light->outerAngle;
        auto light_intensity = light->intensity;

        auto updateCamera = [&](double clip_near_z, double clip_far_z, const dmat4& clipToWorld) -> void {
            const auto& shadowMap = shadowMaps[shadowMapIndex];
            preRenderSwitch->children[shadowMapIndex].mask = MASK_ALL;

            const auto& camera = shadowMap.view->camera;
            auto lookAt = camera->viewMatrix.cast<LookAt>();
            auto relativeProjection = camera->projectionMatrix.cast<RelativeProjection>();

            if (!lookAt) camera->viewMatrix = lookAt = LookAt::create();
            if (!relativeProjection) camera->projectionMatrix = relativeProjection = RelativeProjection::create(dmat4{}, Perspective::create());

            auto perspective = relativeProjection->projectionMatrix.cast<Perspective>();

            lookAt->eye = light_position;
            lookAt->center = light_position + light_z;
            lookAt->up = light_y;

            perspective->aspectRatio = 1.0;
            perspective->fieldOfViewY = 2.0 * degrees(light_outerAngle);
            perspective->farDistance = sqrt(light_intensity / 0.001);
            perspective->nearDistance = perspective->farDistance / 10000.0;

            dmat4 intermediateProjView = perspective->transform() * camera->viewMatrix->transform();

            auto ls_bounds = computeFrustumBoundsClipped(clip_near_z, clip_far_z, intermediateProjView * clipToWorld);
            ls_bounds.min = dvec3(std::max(-1.0, ls_bounds.min.x), std::max(-1.0, ls_bounds.min.y), std::max(0.0, ls_bounds.min.z));
            ls_bounds.max = dvec3(std::min(1.0, ls_bounds.max.x), std::min(1.0, ls_bounds.max.y), std::min(1.0, ls_bounds.max.z));

            // we need to use the reverse Z depth range without actually reversing depth, as the previous matrix already does that
            auto tweakedOrthographic = [](double left, double right, double bottom, double top, double zNear, double zFar) {
                return dmat4(2.0 / (right - left), 0.0, 0.0, 0.0,
                             0.0, 2.0 / (top - bottom), 0.0, 0.0,
                             0.0, 0.0, 1.0 / (zFar - zNear), 0.0,
                             -(right + left) / (right - left), -(top + bottom) / (top - bottom), -zNear / (zFar - zNear), 1.0);
            };

            relativeProjection->matrix = tweakedOrthographic(ls_bounds.min.x, ls_bounds.max.x, ls_bounds.min.y, ls_bounds.max.y, ls_bounds.min.z, ls_bounds.max.z);

            dmat4 shadowMapProjView = camera->projectionMatrix->transform() * camera->viewMatrix->transform();

            dmat4 shadowMapTM = scale(0.5, 0.5, 1.0 + shadowMapBias) * translate(1.0, 1.0, 0.0) * shadowMapProjView * inverse_viewMatrix;

            // convert tex gen matrix to float matrix and assign to light data
            mat4 m(shadowMapTM);

            (*light_itr++) = m[0];
            (*light_itr++) = m[1];
            (*light_itr++) = m[2];
            (*light_itr++) = m[3];

            // info("m = ", m);

            m = inverse(m);

            (*light_itr++) = m[0];
            (*light_itr++) = m[1];
            (*light_itr++) = m[2];
            (*light_itr++) = m[3];

            // advance to the next shadowMap
            shadowMapIndex++;
        };

#if 0
        info("     light_x = ", light_x);
        info("     light_y = ", light_y);
        info("     light_z = ", light_z);
#endif

#if 0
        double range = f - n;
        info("    n = ", n, ", f = ", f, ", range = ", range);
#endif
        auto clipToWorld = inverse(projectionMatrix * viewMatrix);

        if (activeNumShadowMaps > 1)
        {
            double m = static_cast<double>(activeNumShadowMaps);
            for (double i = 0; i < m; i += 1.0)
            {
                dvec3 eye_near(0.0, 0.0, -Cpractical(n, f, i, m, lambda));
                dvec3 eye_far(0.0, 0.0, -Cpractical(n, f, i + 1.0, m, lambda));

                auto clip_near = projectionMatrix * eye_near;
                auto clip_far = projectionMatrix * eye_far;

                updateCamera(clip_near.z, clip_far.z, clipToWorld);
            }
        }
        else
        {
            dvec3 eye_near(0.0, 0.0, -n);
            dvec3 eye_far(0.0, 0.0, -f);

            auto clip_near = projectionMatrix * eye_near;
            auto clip_far = projectionMatrix * eye_far;

            updateCamera(clip_near.z, clip_far.z, clipToWorld);
        }
    }

    if (requiresPerRenderShadowMaps && preRenderCommandGraph)
    {
        if (rt.instrumentation && !preRenderCommandGraph->instrumentation)
        {
            preRenderCommandGraph->instrumentation = shareOrDuplicateForThreadSafety(rt.instrumentation);
        }

        // info("ViewDependentState::traverse(RecordTraversal&) doing pre render command graph. shadowMapIndex = ", shadowMapIndex);
        preRenderCommandGraph->accept(rt);
    }
}

void ViewDependentState::bindDescriptorSets(CommandBuffer& commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet)
{
    auto vk = descriptorSet->vk(commandBuffer.deviceID);
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, 1, &vk, 0, nullptr);
}
