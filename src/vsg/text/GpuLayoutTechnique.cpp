/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/Draw.h>
#include <vsg/core/Array2D.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/text/GpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>

#include <iostream>

#include "shaders/text_GpuLayout_vert.cpp"
#include "shaders/text_frag.cpp"

using namespace vsg;

GpuLayoutTechnique::GpuLayoutState::GpuLayoutState(Font* font)
{
    // load shaders
    auto vertexShader = read_cast<ShaderStage>("shaders/text_GpuLayout.vert", font->options);
    if (!vertexShader) vertexShader = text_GpuLayout_vert(); // fallback to shaders/text_GpuLayout_vert.cppp

    auto fragmentShader = read_cast<ShaderStage>("shaders/text.frag", font->options);
    if (!fragmentShader) fragmentShader = text_frag();

    // compile section
    ShaderStages stagesToCompile;
    if (vertexShader && vertexShader->module && vertexShader->module->code.empty()) stagesToCompile.emplace_back(vertexShader);
    if (fragmentShader && fragmentShader->module && fragmentShader->module->code.empty()) stagesToCompile.emplace_back(fragmentShader);

    uint32_t numTextIndices = 256;
    vertexShader->specializationConstants = vsg::ShaderStage::SpecializationConstants{
        {0, vsg::uintValue::create(numTextIndices)} // numTextIndices
    };

    // Glyph
    DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // texture atlas
        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}    // glyph matrices
    };

    auto descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);

    // set up graphics pipeline
    DescriptorSetLayoutBindings textArrayDescriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}, // Layout uniform
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}  // Text uniform
    };

    textArrayDescriptorSetLayout = DescriptorSetLayout::create(textArrayDescriptorBindings);

    PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
    };

    VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
    };

    VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
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
        InputAssemblyState::create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP),
        MultisampleState::create(),
        blending,
        rasterization,
        DepthStencilState::create()};

    pipelineLayout = PipelineLayout::create(DescriptorSetLayouts{descriptorSetLayout, textArrayDescriptorSetLayout}, pushConstantRanges);

    auto graphicsPipeline = GraphicsPipeline::create(pipelineLayout, ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    bindGraphicsPipeline = BindGraphicsPipeline::create(graphicsPipeline);

    // create texture image and associated DescriptorSets and binding
    auto fontState = font->getShared<Font::FontState>();
    auto descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{fontState->textureAtlas, fontState->glyphMetricsImage});

    bindDescriptorSet = BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSet);
}

template<typename T>
void assignValue(T& dest, const T& src, bool& updated)
{
    if (dest == src) return;
    dest = src;
    updated = true;
}

void GpuLayoutTechnique::setup(Text* text, uint32_t minimumAllocation)
{
    auto layout = text->layout;

    bool textLayoutUpdated = false;
    bool textArrayUpdated = false;

    struct ConvertString : public ConstVisitor
    {
        Font& font;
        ref_ptr<uintArray>& textArray;
        bool& updated;
        uint32_t minimumSize = 0;
        uint32_t size = 0;

        ConvertString(Font& in_font, ref_ptr<uintArray>& in_textArray, bool& in_updated, uint32_t in_minimumSize) :
            font(in_font),
            textArray(in_textArray),
            updated(in_updated),
            minimumSize(in_minimumSize) {}

        void allocate(uint32_t allocationSize)
        {
            size = allocationSize;

            if (allocationSize < minimumSize) allocationSize = minimumSize;
            if (!textArray || allocationSize > static_cast<uint32_t>(textArray->valueCount()))
            {
                updated = true;
                textArray = uintArray::create(allocationSize, 0u);
            }
        }

        void apply(const stringValue& text) override
        {
            allocate(static_cast<uint32_t>(text.value().size()));

            auto itr = textArray->begin();
            for (auto& c : text.value())
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(uint8_t(c)), updated);
            }
        }
        void apply(const ubyteArray& text) override
        {
            allocate(static_cast<uint32_t>(text.valueCount()));

            auto itr = textArray->begin();
            for (auto& c : text)
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(c), updated);
            }
        }
        void apply(const ushortArray& text) override
        {
            allocate(static_cast<uint32_t>(text.valueCount()));

            auto itr = textArray->begin();
            for (auto& c : text)
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(c), updated);
            }
        }
        void apply(const uintArray& text) override
        {
            allocate(static_cast<uint32_t>(text.valueCount()));

            auto itr = textArray->begin();
            for (auto& c : text)
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(c), updated);
            }
        }
    };

    ConvertString convert(*(text->font), textArray, textArrayUpdated, minimumAllocation);
    text->text->accept(convert);

    uint32_t num_quads = convert.size;

    // TODO need to reallocate DescriptorBuffer if textArray changes size?
    if (!textDescriptor) textDescriptor = DescriptorBuffer::create(textArray, 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    // set up the layout data in a form digestible by the GPU.
    if (!layoutValue) layoutValue = TextLayoutValue::create();

    auto& layoutStruct = layoutValue->value();
    if (auto standardLayout = layout.cast<StandardLayout>(); standardLayout)
    {
        assignValue(layoutStruct.position, standardLayout->position, textLayoutUpdated);
        assignValue(layoutStruct.horizontal, standardLayout->horizontal, textLayoutUpdated);
        assignValue(layoutStruct.vertical, standardLayout->vertical, textLayoutUpdated);
        assignValue(layoutStruct.color, standardLayout->color, textLayoutUpdated);
        assignValue(layoutStruct.outlineColor, standardLayout->outlineColor, textLayoutUpdated);
        assignValue(layoutStruct.outlineWidth, standardLayout->outlineWidth, textLayoutUpdated);
    }

    if (!layoutDescriptor) layoutDescriptor = DescriptorBuffer::create(layoutValue, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    if (!vertices)
    {
        vertices = vec3Array::create(4);

        float leadingEdgeGradient = 0.1f;

        vertices->set(0, vec3(0.0f, 1.0f, 2.0f * leadingEdgeGradient));
        vertices->set(1, vec3(0.0f, 0.0f, leadingEdgeGradient));
        vertices->set(2, vec3(1.0f, 1.0f, leadingEdgeGradient));
        vertices->set(3, vec3(1.0f, 0.0f, 0.0f));
    }

    if (!draw)
        draw = Draw::create(4, num_quads, 0, 0);
    else
        draw->instanceCount = num_quads;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    if (!scenegraph)
    {
        scenegraph = StateGroup::create();

        // set up state related objects if they haven't already been assigned
        if (!sharedRenderingState) sharedRenderingState = text->font->getShared<GpuLayoutState>();
        if (sharedRenderingState->bindGraphicsPipeline) scenegraph->add(sharedRenderingState->bindGraphicsPipeline);
        if (sharedRenderingState->bindDescriptorSet) scenegraph->add(sharedRenderingState->bindDescriptorSet);

        // set up graphics pipeline
        auto textDescriptorSet = DescriptorSet::create(sharedRenderingState->textArrayDescriptorSetLayout, Descriptors{layoutDescriptor, textDescriptor});
        bindTextDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, sharedRenderingState->pipelineLayout, 1, textDescriptorSet);
        scenegraph->add(bindTextDescriptorSet);

        bindVertexBuffers = BindVertexBuffers::create(0, DataList{vertices});

        // setup geometry
        auto drawCommands = Commands::create();
        drawCommands->addChild(bindVertexBuffers);
        drawCommands->addChild(draw);

        scenegraph->addChild(drawCommands);
    }
    else
    {
        if (textArrayUpdated)
        {
            textDescriptor->copyDataListToBuffers();
        }
        if (textLayoutUpdated)
        {
            layoutDescriptor->copyDataListToBuffers();
        }
    }
}
