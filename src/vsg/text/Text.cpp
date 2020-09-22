/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/Commands.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/io/read.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/text/Text.h>

#include <iostream>

using namespace vsg;

Text::RenderingState::RenderingState(Font* font)
{
    auto textureData = font->atlas;

    // load shaders
    auto vertexShader = read_cast<ShaderStage>("shaders/text.vert", font->options);
    auto fragmentShader = read_cast<ShaderStage>("shaders/text.frag", font->options);

    if (!vertexShader || !fragmentShader)
    {
        std::cout<<"Could not create shaders."<<std::endl;
        return;
    }

    // compile section
    ShaderStages stagesToCompile;
    if (vertexShader && vertexShader->module && vertexShader->module->code.empty()) stagesToCompile.emplace_back(vertexShader);
    if (fragmentShader && fragmentShader->module && fragmentShader->module->code.empty()) stagesToCompile.emplace_back(fragmentShader);

    // set up graphics pipeline
    DescriptorSetLayoutBindings descriptorBindings
    {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
    };

    auto descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);

    PushConstantRanges pushConstantRanges
    {
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls autoaatically provided by the VSG's DispatchTraversal
    };

    VertexInputState::Bindings vertexBindingsDescriptions
    {
        VkVertexInputBindingDescription{0, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
        VkVertexInputBindingDescription{1, sizeof(vec4), VK_VERTEX_INPUT_RATE_VERTEX}, // colour data
        VkVertexInputBindingDescription{2, sizeof(vec2), VK_VERTEX_INPUT_RATE_VERTEX}  // tex coord data
    };

    VertexInputState::Attributes vertexAttributeDescriptions
    {
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0}, // colour data
        VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},    // tex coord data
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

    GraphicsPipelineStates pipelineStates
    {
        VertexInputState::create( vertexBindingsDescriptions, vertexAttributeDescriptions ),
        InputAssemblyState::create(),
        MultisampleState::create(),
        blending,
        rasterization,
        DepthStencilState::create()
    };

    auto pipelineLayout = PipelineLayout::create(DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    auto graphicsPipeline = GraphicsPipeline::create(pipelineLayout, ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    bindGraphicsPipeline = BindGraphicsPipeline::create(graphicsPipeline);

    // create texture image and associated DescriptorSets and binding
    auto texture = DescriptorImage::create(Sampler::create(), textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    auto descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{texture});

    bindDescriptorSet = BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSet);
}

void Text::setup()
{
    // set up state related objects if they haven't lready been assigned
    if (!_sharedRenderingState) _sharedRenderingState = font->getShared<RenderingState>();

    if (!layout) layout = LeftAlignment::create();

    TextQuads quads;
    layout->layout(text, *font, quads);

    if (quads.empty()) return;


    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = vsg::StateGroup::create();

    _stategroup = scenegraph;

    scenegraph->add(_sharedRenderingState->bindGraphicsPipeline);
    scenegraph->add(_sharedRenderingState->bindDescriptorSet);

    uint32_t num_vertices = quads.size() * 4;
    auto vertices = vsg::vec3Array::create(num_vertices);
    auto colors = vsg::vec4Array::create(num_vertices);
    auto texcoords = vsg::vec2Array::create(num_vertices);
    auto indices = vsg::ushortArray::create(quads.size() * 6);

    uint32_t i = 0;
    uint32_t vi = 0;

    for(auto& quad : quads)
    {
        vertices->set(vi, quad.vertices[0]);
        vertices->set(vi+1, quad.vertices[1]);
        vertices->set(vi+2, quad.vertices[2]);
        vertices->set(vi+3, quad.vertices[3]);

        colors->set(vi, quad.colors[0]);
        colors->set(vi+1, quad.colors[1]);
        colors->set(vi+2, quad.colors[2]);
        colors->set(vi+3, quad.colors[3]);

        texcoords->set(vi, quad.texcoords[0]);
        texcoords->set(vi+1, quad.texcoords[1]);
        texcoords->set(vi+2, quad.texcoords[2]);
        texcoords->set(vi+3, quad.texcoords[3]);

        indices->set(i, vi);
        indices->set(i+1, vi+1);
        indices->set(i+2, vi+2);
        indices->set(i+3, vi+2);
        indices->set(i+4, vi+3);
        indices->set(i+5, vi);

        vi += 4;
        i += 6;
    }

    DataList arrays;

    // setup geometry
    auto drawCommands = vsg::Commands::create();
    drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{vertices, colors, texcoords}));
    drawCommands->addChild(vsg::BindIndexBuffer::create(indices));
    drawCommands->addChild(vsg::DrawIndexed::create(indices->size(), 1, 0, 0, 0));

    scenegraph->addChild(drawCommands);
}
