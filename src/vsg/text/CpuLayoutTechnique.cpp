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
#include <vsg/io/read.h>
#include <vsg/io/write.h>
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

#include "shaders/text_frag.cpp"
#include "shaders/text_vert.cpp"

#include <iostream>

using namespace vsg;

CpuLayoutTechnique::RenderingState::RenderingState(Font* font, bool in_singleColor, bool in_singleOutlineColor, bool in_singleOutlineWidth) :
    singleColor(in_singleColor),
    singleOutlineColor(in_singleOutlineColor),
    singleOutlineWidth(in_singleOutlineWidth)
{
    // load shaders
    auto vertexShader = read_cast<ShaderStage>("shaders/text.vert", font->options);
    if (!vertexShader) vertexShader = text_vert(); // fallback to shaders/text_vert.cppp

    auto fragmentShader = read_cast<ShaderStage>("shaders/text.frag", font->options);
    if (!fragmentShader) fragmentShader = text_frag(); // fallback to shaders/text_frag.cppp

    // compile section
    ShaderStages stagesToCompile;
    if (vertexShader && vertexShader->module && vertexShader->module->code.empty()) stagesToCompile.emplace_back(vertexShader);
    if (fragmentShader && fragmentShader->module && fragmentShader->module->code.empty()) stagesToCompile.emplace_back(fragmentShader);

    // set up graphics pipeline
    DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
    };

    auto descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);

    PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
    };

    VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX},                                                       // vertex data
        VkVertexInputBindingDescription{1, sizeof(vec4), singleColor ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX},         // colour data
        VkVertexInputBindingDescription{2, sizeof(vec4), singleOutlineColor ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX},  // outline colour data
        VkVertexInputBindingDescription{3, sizeof(float), singleOutlineWidth ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX}, // outline width data
        VkVertexInputBindingDescription{4, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX}                                                        // tex coord data
    };

    VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},    // vertex data
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0}, // colour data
        VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32B32A32_SFLOAT, 0}, // outline colour data
        VkVertexInputAttributeDescription{3, 3, VK_FORMAT_R32_SFLOAT, 0},          // outline width data
        VkVertexInputAttributeDescription{4, 4, VK_FORMAT_R32G32B32_SFLOAT, 0},    // tex coord data
    };

    // alpha blending
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
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    auto blending = ColorBlendState::create(ColorBlendState::ColorBlendAttachments{colorBlendAttachment});

    // switch off back face culling
    auto rasterization = RasterizationState::create();
    rasterization->cullMode = VK_CULL_MODE_NONE;

    GraphicsPipelineStates pipelineStates{
        VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        InputAssemblyState::create(),
        MultisampleState::create(),
        blending,
        rasterization,
        DepthStencilState::create()};

    auto pipelineLayout = PipelineLayout::create(DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    auto graphicsPipeline = GraphicsPipeline::create(pipelineLayout, ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    bindGraphicsPipeline = BindGraphicsPipeline::create(graphicsPipeline);

    // create texture image and associated DescriptorSets and binding
    auto fontState = font->getShared<Font::FontState>();
    auto descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{fontState->textureAtlas});
    bindDescriptorSet = BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSet);
}

void CpuLayoutTechnique::setup(Text* text, uint32_t minimumAllocation)
{
    auto layout = text->layout;

    TextQuads quads;
    layout->layout(text->text, *(text->font), quads);

    vec4 color = quads.front().colors[0];
    vec4 outlineColor = quads.front().outlineColors[0];
    float outlineWidth = quads.front().outlineWidths[0];
    bool singleColor = true;
    bool singleOutlineColor = true;
    bool singleOutlineWidth = true;
    for (auto& quad : quads)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (quad.colors[i] != color) singleColor = false;
            if (quad.outlineColors[i] != outlineColor) singleOutlineColor = false;
            if (quad.outlineWidths[i] != outlineWidth) singleOutlineWidth = false;
        }
    }

    uint32_t num_quads = std::max(static_cast<uint32_t>(quads.size()), minimumAllocation);
    uint32_t num_vertices = num_quads * 4;
    uint32_t num_colors = singleColor ? 1 : num_vertices;
    uint32_t num_outlineColors = singleOutlineColor ? 1 : num_vertices;
    uint32_t num_outlineWidths = singleOutlineWidth ? 1 : num_vertices;

    if (!vertices || num_vertices > vertices->size()) vertices = vec3Array::create(num_vertices);
    if (!colors || num_colors > colors->size()) colors = vec4Array::create(num_colors);
    if (!outlineColors || num_outlineColors > outlineColors->size()) outlineColors = vec4Array::create(num_outlineColors);
    if (!outlineWidths || num_outlineWidths > outlineWidths->size()) outlineWidths = floatArray::create(num_outlineWidths);
    if (!texcoords || num_vertices > texcoords->size()) texcoords = vec3Array::create(num_vertices);

    uint32_t vi = 0;

    float leadingEdgeGradient = 0.1f;

    if (singleColor) colors->set(0, color);
    if (singleOutlineColor) outlineColors->set(0, outlineColor);
    if (singleOutlineWidth) outlineWidths->set(0, outlineWidth);

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
    if (!scenegraph)
    {
        scenegraph = StateGroup::create();

        // set up state related objects if they haven't already been assigned
        if (!sharedRenderingState) sharedRenderingState = text->font->getShared<RenderingState>(singleColor, singleOutlineColor, singleOutlineWidth);

        if (sharedRenderingState->bindGraphicsPipeline) scenegraph->add(sharedRenderingState->bindGraphicsPipeline);
        if (sharedRenderingState->bindDescriptorSet) scenegraph->add(sharedRenderingState->bindDescriptorSet);

        bindVertexBuffers = BindVertexBuffers::create(0, DataList{vertices, colors, outlineColors, outlineWidths, texcoords});
        bindIndexBuffer = BindIndexBuffer::create(indices);

        // setup geometry
        auto drawCommands = Commands::create();
        drawCommands->addChild(bindVertexBuffers);
        drawCommands->addChild(bindIndexBuffer);
        drawCommands->addChild(drawIndexed);

        scenegraph->addChild(drawCommands);
    }
    else
    {
        std::cout << "TODO : CpuLayoutTechnique::setup(), does not yet support updates. Consider using GpuLayoutTechnique instead." << std::endl;
        // bindVertexBuffers->copyDataToBuffers();
        // bindIndexBuffer->copyDataToBuffers();
    }
}
