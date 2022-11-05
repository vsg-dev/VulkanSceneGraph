/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/io/Logger.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/text/CpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>
#include <vsg/text/TextGroup.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

void CpuLayoutTechnique::setup(Text* text, uint32_t minimumAllocation, ref_ptr<const Options> options)
{
    if (!text || !(text->text) || !text->font || !text->layout) return;

    const auto& font = text->font;
    auto& layout = text->layout;
    auto shaderSet = text->shaderSet ? text->shaderSet : createTextShaderSet(options);

    textExtents = layout->extents(text->text, *(text->font));

    auto num_quads = vsg::visit<CountGlyphs>(text->text).count;

    TextQuads quads;
    quads.reserve(num_quads);
    layout->layout(text->text, *font, quads);

    scenegraph = createRenderingSubgraph(shaderSet, font, layout->requiresBillboard(), quads, minimumAllocation);
}

void CpuLayoutTechnique::setup(TextGroup* textGroup, uint32_t minimumAllocation, ref_ptr<const Options> options)
{
    if (!textGroup || textGroup->children.empty()) return;

    const auto& font = textGroup->font;
    auto shaderSet = textGroup->shaderSet ? textGroup->shaderSet : createTextShaderSet(options);

    auto& first_text = textGroup->children.front();
    auto& layout = first_text->layout;
    bool requiresBillboard = layout ? layout->requiresBillboard() : false;

    textExtents = {};
    CountGlyphs countGlyphs;
    for (auto& text : textGroup->children)
    {
        if (text->text && text->layout)
        {
            text->text->accept(countGlyphs);
            textExtents.add(text->layout->extents(text->text, *(font)));
        }
    }

    TextQuads quads;
    quads.reserve(countGlyphs.count);
    for (auto& text : textGroup->children)
    {
        if (text->text && text->layout) text->layout->layout(text->text, *font, quads);
    }

    scenegraph = createRenderingSubgraph(shaderSet, font, requiresBillboard, quads, minimumAllocation);
}

ref_ptr<Node> CpuLayoutTechnique::createRenderingSubgraph(ref_ptr<ShaderSet> shaderSet, ref_ptr<Font> font, bool billboard, TextQuads& quads, uint32_t minimumAllocation)
{
    if (quads.empty()) return {};

    ref_ptr<StateGroup> stategroup;

    vec4 color = quads.front().colors[0];
    vec4 outlineColor = quads.front().outlineColors[0];
    float outlineWidth = quads.front().outlineWidths[0];
    vec4 centerAndAutoScaleDistance = quads.front().centerAndAutoScaleDistance;
    bool singleColor = true;
    bool singleOutlineColor = true;
    bool singleOutlineWidth = true;
    bool singleCenterAutoScaleDistance = true;
    for (auto& quad : quads)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (quad.colors[i] != color) singleColor = false;
            if (quad.outlineColors[i] != outlineColor) singleOutlineColor = false;
            if (quad.outlineWidths[i] != outlineWidth) singleOutlineWidth = false;
        }
        if (quad.centerAndAutoScaleDistance != centerAndAutoScaleDistance) singleCenterAutoScaleDistance = false;
    }

    uint32_t num_quads = std::max(static_cast<uint32_t>(quads.size()), minimumAllocation);
    uint32_t num_vertices = num_quads * 4;
    uint32_t num_colors = singleColor ? 1 : num_vertices;
    uint32_t num_outlineColors = singleOutlineColor ? 1 : num_vertices;
    uint32_t num_outlineWidths = singleOutlineWidth ? 1 : num_vertices;
    uint32_t num_centerAndAutoScaleDistances = billboard ? (singleCenterAutoScaleDistance ? 1 : num_vertices) : 0;

    if (!vertices || num_vertices > vertices->size()) vertices = vec3Array::create(num_vertices);
    if (!colors || num_colors > colors->size()) colors = vec4Array::create(num_colors);
    if (!outlineColors || num_outlineColors > outlineColors->size()) outlineColors = vec4Array::create(num_outlineColors);
    if (!outlineWidths || num_outlineWidths > outlineWidths->size()) outlineWidths = floatArray::create(num_outlineWidths);
    if (!texcoords || num_vertices > texcoords->size()) texcoords = vec3Array::create(num_vertices);
    if (billboard && (!centerAndAutoScaleDistances || num_centerAndAutoScaleDistances > centerAndAutoScaleDistances->size())) centerAndAutoScaleDistances = vec4Array::create(num_centerAndAutoScaleDistances);

    uint32_t vi = 0;

    float leadingEdgeGradient = 0.1f;

    if (singleColor) colors->set(0, color);
    if (singleOutlineColor) outlineColors->set(0, outlineColor);
    if (singleOutlineWidth) outlineWidths->set(0, outlineWidth);
    if (singleCenterAutoScaleDistance && centerAndAutoScaleDistances) centerAndAutoScaleDistances->set(0, centerAndAutoScaleDistance);

    for (auto& quad : quads)
    {
        float leadingEdgeTilt = length(quad.vertices[0] - quad.vertices[1]) * leadingEdgeGradient;
        float topEdgeTilt = leadingEdgeTilt;

        vertices->set(vi, quad.vertices[0]);
        vertices->set(vi + 1, quad.vertices[1]);
        vertices->set(vi + 2, quad.vertices[2]);
        vertices->set(vi + 3, quad.vertices[3]);

        if (!singleColor)
        {
            colors->set(vi, quad.colors[0]);
            colors->set(vi + 1, quad.colors[1]);
            colors->set(vi + 2, quad.colors[2]);
            colors->set(vi + 3, quad.colors[3]);
        }

        if (!singleOutlineColor)
        {
            outlineColors->set(vi, quad.outlineColors[0]);
            outlineColors->set(vi + 1, quad.outlineColors[1]);
            outlineColors->set(vi + 2, quad.outlineColors[2]);
            outlineColors->set(vi + 3, quad.outlineColors[3]);
        }

        if (!singleOutlineWidth)
        {
            outlineWidths->set(vi, quad.outlineWidths[0]);
            outlineWidths->set(vi + 1, quad.outlineWidths[1]);
            outlineWidths->set(vi + 2, quad.outlineWidths[2]);
            outlineWidths->set(vi + 3, quad.outlineWidths[3]);
        }

        texcoords->set(vi, vec3(quad.texcoords[0].x, quad.texcoords[0].y, leadingEdgeTilt + topEdgeTilt));
        texcoords->set(vi + 1, vec3(quad.texcoords[1].x, quad.texcoords[1].y, topEdgeTilt));
        texcoords->set(vi + 2, vec3(quad.texcoords[2].x, quad.texcoords[2].y, 0.0f));
        texcoords->set(vi + 3, vec3(quad.texcoords[3].x, quad.texcoords[3].y, leadingEdgeTilt));

        if (!singleCenterAutoScaleDistance && centerAndAutoScaleDistances)
        {
            centerAndAutoScaleDistances->set(vi, quad.centerAndAutoScaleDistance);
            centerAndAutoScaleDistances->set(vi + 1, quad.centerAndAutoScaleDistance);
            centerAndAutoScaleDistances->set(vi + 2, quad.centerAndAutoScaleDistance);
            centerAndAutoScaleDistances->set(vi + 3, quad.centerAndAutoScaleDistance);
        }

        vi += 4;
    }

    uint32_t num_indices = num_quads * 6;
    if (!indices || num_indices > indices->valueCount())
    {
        if (num_vertices > 65536) // check if requires uint or ushort indices
        {
            auto ui_indices = uintArray::create(num_indices);
            indices = ui_indices;

            auto itr = ui_indices->begin();
            vi = 0;
            for (uint32_t i = 0; i < num_quads; ++i)
            {
                (*itr++) = vi;
                (*itr++) = vi + 1;
                (*itr++) = vi + 2;
                (*itr++) = vi + 2;
                (*itr++) = vi + 3;
                (*itr++) = vi;

                vi += 4;
            }
        }
        else
        {
            auto us_indices = ushortArray::create(num_indices);
            indices = us_indices;

            auto itr = us_indices->begin();
            vi = 0;
            for (uint32_t i = 0; i < num_quads; ++i)
            {
                (*itr++) = vi;
                (*itr++) = vi + 1;
                (*itr++) = vi + 2;
                (*itr++) = vi + 2;
                (*itr++) = vi + 3;
                (*itr++) = vi;

                vi += 4;
            }
        }
    }

    if (!drawIndexed)
        drawIndexed = DrawIndexed::create(static_cast<uint32_t>(quads.size() * 6), 1, 0, 0, 0);
    else
        drawIndexed->indexCount = static_cast<uint32_t>(quads.size() * 6);

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    if (!stategroup)
    {
        stategroup = StateGroup::create();

        auto config = vsg::GraphicsPipelineConfigurator::create(shaderSet);

        auto& sharedObjects = font->sharedObjects;
        if (!sharedObjects) sharedObjects = SharedObjects::create();

        DataList arrays;
        config->assignArray(arrays, "inPosition", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
        config->assignArray(arrays, "inColor", singleColor ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX, colors);
        config->assignArray(arrays, "inOutlineColor", singleOutlineColor ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX, outlineColors);
        config->assignArray(arrays, "inOutlineWidth", singleOutlineWidth ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX, outlineWidths);
        config->assignArray(arrays, "inTexCoord", VK_VERTEX_INPUT_RATE_VERTEX, texcoords);

        if (centerAndAutoScaleDistances)
        {
            config->assignArray(arrays, "inCenterAndAutoScaleDistance", singleCenterAutoScaleDistance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX, centerAndAutoScaleDistances);
        }

        // set up sampler for atlas.
        auto sampler = Sampler::create();
        sampler->magFilter = VK_FILTER_LINEAR;
        sampler->minFilter = VK_FILTER_LINEAR;
        sampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler->borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        sampler->anisotropyEnable = VK_TRUE;
        sampler->maxAnisotropy = 16.0f;
        sampler->maxLod = 12.0;

        if (sharedObjects) sharedObjects->share(sampler);

        Descriptors descriptors;
        config->assignTexture(descriptors, "textureAtlas", font->atlas, sampler);
        if (sharedObjects) sharedObjects->share(descriptors);

        // disable face culling so text can be seen from both sides
        config->rasterizationState->cullMode = VK_CULL_MODE_NONE;

        // set alpha blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                              VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT |
                                              VK_COLOR_COMPONENT_A_BIT;

        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        config->colorBlendState->attachments = {colorBlendAttachment};

        if (sharedObjects)
            sharedObjects->share(config, [](auto gpc) { gpc->init(); });
        else
            config->init();

        stategroup->add(config->bindGraphicsPipeline);

        auto descriptorSetLayout = vsg::DescriptorSetLayout::create(config->descriptorBindings);
        if (sharedObjects) sharedObjects->share(descriptorSetLayout);
        auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, descriptors);

        auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, config->layout, 0, descriptorSet);
        if (sharedObjects) sharedObjects->share(bindDescriptorSet);

        stategroup->add(bindDescriptorSet);

        bindVertexBuffers = BindVertexBuffers::create(0, arrays);
        bindIndexBuffer = BindIndexBuffer::create(indices);

        // setup geometry
        auto drawCommands = Commands::create();
        drawCommands->addChild(bindVertexBuffers);
        drawCommands->addChild(bindIndexBuffer);
        drawCommands->addChild(drawIndexed);

        stategroup->addChild(drawCommands);
    }
    else
    {
        info("TODO : CpuLayoutTechnique::setup(), does not yet support updates. Consider using GpuLayoutTechnique instead.");
        // bindVertexBuffers->copyDataToBuffers();
        // bindIndexBuffer->copyDataToBuffers();
    }

    return stategroup;
}
