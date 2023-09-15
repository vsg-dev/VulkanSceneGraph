/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/io/write.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

//////////////////////////////////////
//
// TraverseChildrenOfNode
//
namespace vsg
{
    class TraverseChildrenOfNode : public vsg::Inherit<vsg::Node, TraverseChildrenOfNode>
    {
    public:
        explicit TraverseChildrenOfNode(vsg::Node* in_node) :
            node(in_node) {}

        vsg::observer_ptr<vsg::Node> node;

        template<class N, class V>
        static void t_traverse(N& in_node, V& visitor)
        {
            if (auto ref_node = in_node.node.ref_ptr()) ref_node->traverse(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }
    };
    VSG_type_name(vsg::TraverseChildrenOfNode);
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

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
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

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

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
ViewDependentState::ViewDependentState(View* in_view, bool in_active) :
    view(in_view),
    active(in_active)
{
}

ViewDependentState::~ViewDependentState()
{
}

void ViewDependentState::init(ResourceRequirements& requirements)
{
    uint32_t maxNumberLights = 64;
    uint32_t maxViewports = 1;
    uint32_t maxShadowMaps = 8;

    info("ViewDependentState::init() ", maxNumberLights, ", ", maxViewports, ", this = ", this, ", acrive = ", active);

    lightData = vec4Array::create(maxNumberLights);
    lightData->properties.dataVariance = DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
    lightDataBufferInfo = BufferInfo::create(lightData.get());

    viewportData = vec4Array::create(maxViewports);
    viewportData->properties.dataVariance = DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
    viewportDataBufferInfo = BufferInfo::create(viewportData.get());

    descriptor = DescriptorBuffer::create(BufferInfoList{lightDataBufferInfo, viewportDataBufferInfo}, 0); // hardwired position for now

    // set up ShadowMaps
    auto shadwoMapSampler = vsg::Sampler::create();
    shadwoMapSampler->minFilter = VK_FILTER_NEAREST;
    shadwoMapSampler->magFilter = VK_FILTER_NEAREST;
    shadwoMapSampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadwoMapSampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadwoMapSampler->anisotropyEnable = VK_FALSE;
    shadwoMapSampler->maxAnisotropy = 1;
    shadwoMapSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    // image->imageType = VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D?
    // imageView->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY
    shadowMapData = floatArray3D::create(2048, 2048, maxShadowMaps, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT});
    shadowMapImages = DescriptorImage::create(shadwoMapSampler, shadowMapData, 2);

    DescriptorSetLayoutBindings descriptorBindings{
        VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // lightData
        VkDescriptorSetLayoutBinding{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // viewportData
        VkDescriptorSetLayoutBinding{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };

    descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);
    descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{descriptor, shadowMapImages});
#if 0
    for (uint32_t k = 0; k < shadowMapData->depth(); ++k)
    {
        for (uint32_t j = 0; j < shadowMapData->height(); ++j)
        {
            float* data = shadowMapData->data(shadowMapData->index(0, j, k));
            for (uint32_t i = 0; i < shadowMapData->width(); ++i)
            {
                *(data++) = static_cast<float>(sin(vsg::PI * static_cast<double>(i) / static_cast<double>(shadowMapData->width() - 1)));
            }
        }
    }
#endif
    // if not active then don't enable shadow maps
    if (!active) return;

    // create a switch to toggle on/off the render to texture subgraphs for each shadowmap layer
    preRenderSwitch = Switch::create();

    preRenderCommandGraph = CommandGraph::create();
    preRenderCommandGraph->submitOrder = -1;
    preRenderCommandGraph->addChild(preRenderSwitch);

    auto tcon = TraverseChildrenOfNode::create(view);

    Mask shadowMask = 0x1; // TODO: do we inherit from main scene? how?

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
            first_view = View::create(false);
            shadowMap.view = first_view;
        }

        shadowMap.view->mask = shadowMask;
        shadowMap.view->camera = Camera::create();
        shadowMap.view->addChild(tcon);

        shadowMap.renderGraph = RenderGraph::create();
        shadowMap.renderGraph->addChild(shadowMap.view);
        preRenderSwitch->addChild(MASK_ALL, shadowMap.renderGraph);
    }

    //vsg::write(shadowMapData, "test.vsgt");
}

vsg::ref_ptr<vsg::Image> createShadowImage(vsg::Context& context, uint32_t width, uint32_t height, uint32_t levels, VkFormat format, VkImageUsageFlags usage)
{
    auto image = vsg::Image::create();
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

    // allocte vkImage and required memory
    image->compile(context);

    return image;
}

void ViewDependentState::compile(Context& context)
{
    info("ViewDependentState::compile( ", &context, " ) ", this);

    descriptorSet->compile(context);

    if (active && preRenderCommandGraph && !preRenderCommandGraph->device)
    {

        preRenderCommandGraph->device = context.device;

        auto& resourceRequirements = context.resourceRequirements;
        auto& viewDetails = resourceRequirements.views[view];
        info("   assigning device to preCommandGraph ", preRenderCommandGraph->device);
        info("   assigning device to preRenderCommandGraph->queueFamily =  ", preRenderCommandGraph->queueFamily);
        info("   resourceRequirements.numLightsRange = ", resourceRequirements.numLightsRange);
        info("   resourceRequirements.numShadowMapsRange = ", resourceRequirements.numShadowMapsRange);
        info("   resourceRequirements.shadowMapSize = ", resourceRequirements.shadowMapSize);
        info("   viewDetails.indices.size() = ", viewDetails.indices.size());
        info("   viewDetails.bins.size() = ", viewDetails.bins.size());
        info("   viewDetails.lights.size() = ", viewDetails.lights.size());
        info("   resourceRequirements.numLightsRange = ", resourceRequirements.numLightsRange);
        info("   resourceRequirements.numShadowMapsRange = ", resourceRequirements.numShadowMapsRange);

        // TODO
        preRenderCommandGraph->queueFamily = 0;

        VkExtent2D extent{2048, 2048};
        uint32_t numLayers = shadowMaps.size();

        shadowColorImage = createShadowImage(context, extent.width, extent.height, numLayers, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        shadowDepthImage = createShadowImage(context, extent.width, extent.height, numLayers, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        shadowColorImage->compile(context);
        shadowDepthImage->compile(context);

        uint32_t layer = 0;
        for(auto& shadowMap : shadowMaps)
        {
            auto colorImageView = vsg::ImageView::create(shadowColorImage, VK_IMAGE_ASPECT_COLOR_BIT);
            colorImageView->subresourceRange.baseArrayLayer = layer;
            colorImageView->subresourceRange.layerCount = 1;
            colorImageView->compile(context);

            // Sampler for accessing attachment as a texture
            auto colorSampler = vsg::Sampler::create();
            colorSampler->flags = 0;
            colorSampler->magFilter = VK_FILTER_LINEAR;
            colorSampler->minFilter = VK_FILTER_LINEAR;
            colorSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            colorSampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            colorSampler->addressModeV = colorSampler->addressModeU;
            colorSampler->addressModeW = colorSampler->addressModeU;
            colorSampler->mipLodBias = 0.0f;
            colorSampler->maxAnisotropy = 1.0f;
            colorSampler->minLod = 0.0f;
            colorSampler->maxLod = 1.0f;
            colorSampler->borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

            auto colorImageInfo = ImageInfo::create();
            colorImageInfo->imageView = colorImageView;
            colorImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            colorImageInfo->sampler = colorSampler;

            // create depth buffer

            auto depthImageView = vsg::ImageView::create(shadowDepthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
            depthImageView->subresourceRange.baseArrayLayer = layer;
            depthImageView->subresourceRange.layerCount = 1;
            depthImageView->compile(context);

            auto depthImageInfo = ImageInfo::create();
            depthImageInfo->sampler = nullptr;
            depthImageInfo->imageView = depthImageView;
            depthImageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            // attachment descriptions
            vsg::RenderPass::Attachments attachments(2);
            // Color attachment
            attachments[0].format = shadowColorImage->format;
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            // Depth attachment
            attachments[1].format = shadowDepthImage->format;
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            vsg::AttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
            vsg::AttachmentReference depthReference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
            vsg::RenderPass::Subpasses subpassDescription(1);
            subpassDescription[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription[0].colorAttachments.emplace_back(colorReference);
            subpassDescription[0].depthStencilAttachments.emplace_back(depthReference);

            vsg::RenderPass::Dependencies dependencies(2);

            // XXX This dependency is copied from the offscreenrender.cpp
            // example. I don't completely understand it, but I think its
            // purpose is to create a barrier if some earlier render pass was
            // using this framebuffer's attachment as a texture.
            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            // This is the heart of what makes Vulkan offscreen rendering
            // work: render passes that follow are blocked from using this
            // passes' color attachment in their fragment shaders until all
            // this pass' color writes are finished.
            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            auto renderPass = vsg::RenderPass::create(context.device, attachments, subpassDescription, dependencies);

            // Framebuffer
            auto fbuf = vsg::Framebuffer::create(renderPass, vsg::ImageViews{colorImageInfo->imageView, depthImageInfo->imageView}, extent.width, extent.height, 1);

            auto rendergraph = shadowMap.renderGraph;
            rendergraph->renderArea.offset = VkOffset2D{0, 0};
            rendergraph->renderArea.extent = extent;
            rendergraph->framebuffer = fbuf;

            rendergraph->clearValues.resize(2);
            rendergraph->clearValues[0].color = {{0.4f, 0.2f, 0.4f, 1.0f}};
            rendergraph->clearValues[1].depthStencil = VkClearDepthStencilValue{0.0f, 0};

            // assign to ShadowMap struct
            shadowMap.colorImageInfo = colorImageInfo;
            shadowMap.depthImageInfo = depthImageInfo;

            ++layer;
        }
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
    if (!active || !preRenderSwitch) return;

    bool requiresPerRenderShadowMaps = false;
    preRenderSwitch->setAllChildren(false);

    // useful reference : https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps
    // PCF filtering : https://github.com/SaschaWillems/Vulkan/issues/231
    // sampler2DArrayShadow
    // https://ogldev.org/www/tutorial42/tutorial42.html

    info("\n\nViewDependentState::traverse(", &rt, ", ", &view, ")");
    uint32_t shadowMapIndex = 0;
    uint32_t numShadowMaps = static_cast<uint32_t>(shadowMaps.size());

    for (auto& [mv, light] : directionalLights)
    {
        if (light->shadowMaps == 0) continue;
        if (shadowMapIndex >= numShadowMaps) continue;

        requiresPerRenderShadowMaps = true;

        // compute directional light space
        auto projectionMatrix = view->camera->projectionMatrix->transform();
        auto viewMatrix = view->camera->viewMatrix->transform();

        // view direction in world coords
        auto view_direction = normalize(dvec3(0.0, 0.0, -1.0) * (projectionMatrix * viewMatrix));
        auto view_up = normalize(dvec3(0.0, -1.0, 0.0) * (projectionMatrix * viewMatrix));

        // light direction in world coords
        auto light_direction = normalize(light->direction * (inverse_3x3(mv * inverse(viewMatrix))));

        info("   directional light : light direction in world = ", light_direction, ", light->shadowMaps = ", light->shadowMaps);
        info("      light->direction in model = ", light->direction);
        info("      view_direction in world = ", view_direction);
        info("      view_up in world = ", view_up);

        auto light_x_direction = cross(light_direction, view_direction);
        auto light_x_up = cross(light_direction, view_up);

        auto light_x = (length(light_x_direction) > length(light_x_up)) ? normalize(light_x_direction) : normalize(light_x_up);
        auto light_y = cross(light_x, light_direction);
        auto light_z = light_direction;

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

        auto Clog = [](double n, double f, double i, double m) -> double {
            return n * std::pow((f / n), (i / m));
        };

        auto Cuniform = [](double n, double f, double i, double m) -> double {
            return n + (f - n) * (i / m);
        };

        auto Cpractical = [&Clog, &Cuniform](double n, double f, double i, double m, double lambda) -> double {
            return Clog(n, f, i, m) * lambda + Cuniform(n, f, i, m) * (1.0 - lambda);
        };

        auto updateCamera = [&](double clip_near_z, double clip_far_z, const dmat4& clipToWorld) -> void {

            auto& shadowMap = shadowMaps[shadowMapIndex];
            preRenderSwitch->children[shadowMapIndex].mask = MASK_ALL;

            auto& camera = shadowMap.view->camera;
            auto lookAt = camera->viewMatrix.cast<LookAt>();
            auto ortho = camera->projectionMatrix.cast<Orthographic>();

            info("   lookAt = ", lookAt);
            info("   ortho = ", ortho);

            auto ws_bounds = computeFrustumBounds(clip_near_z, clip_far_z, clipToWorld);
            auto center = (ws_bounds.min + ws_bounds.max) * 0.5;

            if (!lookAt)
            {
                lookAt = vsg::LookAt::create(center, center + light_z, light_y);
                camera->viewMatrix = lookAt;
            }
            else
            {
                lookAt->eye = center;
                lookAt->center = center + light_z;
                lookAt->up = light_y;
            }

            auto ls_bounds = computeFrustumBounds(clip_near_z, clip_far_z, lookAt->transform() * clipToWorld);
            if (!ortho)
            {
                ortho = Orthographic::create(ls_bounds.min.x, ls_bounds.max.x,
                                            ls_bounds.min.y, ls_bounds.max.y,
                                            ls_bounds.min.z, ls_bounds.max.z);
                camera->projectionMatrix = ortho;
            }
            else
            {
                ortho->left = ls_bounds.min.x;
                ortho->right = ls_bounds.max.x;
                ortho->bottom = ls_bounds.min.y;
                ortho->top = ls_bounds.max.y;
                ortho->nearDistance = ls_bounds.min.z;
                ortho->farDistance = ls_bounds.max.z;
            }

            shadowMapIndex++;
        };

        info("     light_x = ", light_x);
        info("     light_y = ", light_y);
        info("     light_z = ", light_z);

        auto clipToEye = inverse(projectionMatrix);

        auto n = -(clipToEye * dvec3(0.0, 0.0, 1.0)).z;
        auto f = -(clipToEye * dvec3(0.0, 0.0, 0.0)).z;

#    if 1
        // clamp the near and far values
        double maxShadowDistance = 1000.0;
        if (n > maxShadowDistance)
        {
            info("Oopps near is further than the maxShadowDistance!");
            n = maxShadowDistance * 0.5;
            f = maxShadowDistance;
        }
        if (f > maxShadowDistance)
        {
            f = maxShadowDistance;
        }
#    endif
        double range = f - n;
        info("    n = ", n, ", f = ", f, ", range = ", range);

        auto clipToWorld = inverse(projectionMatrix * viewMatrix);

        uint32_t numShadowMapsForThisLight = std::min(light->shadowMaps, numShadowMaps - shadowMapIndex);

        if (numShadowMapsForThisLight > 1)
        {
            double lambda = 0.5;
            double m = static_cast<double>(numShadowMapsForThisLight);
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
            updateCamera(1.0, 0.0, clipToWorld);
        }
    }

#if 1
    for (auto& [mv, light] : pointLights)
    {
        auto eye_position = mv * light->position;
        info("   positional light : position = ", eye_position, ", light->shadowMaps = ", light->shadowMaps);
    }

    for (auto& [mv, light] : spotLights)
    {
        auto eye_position = mv * light->position;
        auto eye_direction1 = normalize(light->direction * inverse_3x3(mv));
        info("   spot light : light->direction = ", light->direction, ", position = ", eye_position, ", direction = ", eye_direction1, ", light->shadowMaps = ", light->shadowMaps);
    }
#endif

    // traverse pre render graph
    if (requiresPerRenderShadowMaps && preRenderCommandGraph)
    {
        info("ViewDependentState::traverse(RecordTraversal&) doing pre render command graph. shadowMapIndex = ", shadowMapIndex);
        preRenderCommandGraph->accept(rt);
    }
    else
    {
        info("ViewDependentState::traverse(RecordTraversal&) no need for pre render command graph.");
    }
}

void ViewDependentState::pack()
{
    //debug("ViewDependentState::pack() ambient ", ambientLights.size(), ", diffuse ", directionalLights.size(), ", point ", pointLights.size(), ", spot ", spotLights.size());

    auto light_itr = lightData->begin();
    lightData->dirty();

    (*light_itr++) = vec4(static_cast<float>(ambientLights.size()),
                          static_cast<float>(directionalLights.size()),
                          static_cast<float>(pointLights.size()),
                          static_cast<float>(spotLights.size()));

    // lightData requirements = vec4 * (num_ambientLights + 3 * num_directionLights + 3 * num_pointLights + 4 * num_spotLights + 4 * num_shadow_maps)

    for (auto& entry : ambientLights)
    {
        auto light = entry.second;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
    }

    for (auto& [mv, light] : directionalLights)
    {
        auto eye_direction = normalize(inverse_3x3(mv) * light->direction);
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_direction.x), static_cast<float>(eye_direction.y), static_cast<float>(eye_direction.z), 0.0f);
        (*light_itr++).set(static_cast<float>(light->shadowMaps), 0.0f, 0.0f, 0.0f); // shadow map setting
    }

    for (auto& [mv, light] : pointLights)
    {
        auto eye_position = mv * light->position;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_position.x), static_cast<float>(eye_position.y), static_cast<float>(eye_position.z), 0.0f);
        (*light_itr++).set(static_cast<float>(light->shadowMaps), 0.0f, 0.0f, 0.0f); // shadow map setting
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
        (*light_itr++).set(static_cast<float>(light->shadowMaps), 0.0f, 0.0f, 0.0f); // shadow map setting
    }
#if 0
    for(auto itr = lightData->begin(); itr != light_itr; ++itr)
    {
        debug("   ", *itr);
    }
#endif
}

void ViewDependentState::bindDescriptorSets(CommandBuffer& commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet)
{
    auto vk = descriptorSet->vk(commandBuffer.deviceID);
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, 1, &vk, 0, nullptr);
}
