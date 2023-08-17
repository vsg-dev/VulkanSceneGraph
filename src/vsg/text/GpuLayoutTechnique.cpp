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
#include <vsg/text/GpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

template<typename T>
void assignValue(T& dest, const T& src, bool& updated)
{
    if (dest == src) return;
    dest = src;
    updated = true;
}

void GpuLayoutTechnique::setup(Text* text, uint32_t minimumAllocation, ref_ptr<const Options> options)
{
    auto layout = text->layout;
    if (!layout) return;

    if (!text->font) return;

    textExtents = layout->extents(text->text, *(text->font));

    bool textLayoutUpdated = false;
    bool textArrayUpdated = false;

    struct ConvertString : public ConstVisitor
    {
        Font& font;
        ref_ptr<uintArray>& textArray;
        bool& updated;
        uint32_t minimumSize = 0;
        uint32_t allocatedSize = 0;
        uint32_t size = 0;

        ConvertString(Font& in_font, ref_ptr<uintArray>& in_textArray, bool& in_updated, uint32_t in_minimumSize) :
            font(in_font),
            textArray(in_textArray),
            updated(in_updated),
            minimumSize(in_minimumSize) {}

        void allocate(uint32_t requiredSize)
        {
            size = requiredSize;

            if (textArray && requiredSize < static_cast<uint32_t>(textArray->valueCount()))
            {
                allocatedSize = static_cast<uint32_t>(textArray->valueCount());
                textArray->dirty();
                return;
            }

            allocatedSize = std::max(requiredSize, minimumSize);
            textArray = uintArray::create(allocatedSize, 0u);
            textArray->properties.dataVariance = DYNAMIC_DATA;

            updated = true;
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
        void apply(const wstringValue& text) override
        {
            allocate(static_cast<uint32_t>(text.value().size()));

            auto itr = textArray->begin();
            for (auto& c : text.value())
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(uint16_t(c)), updated);
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

    if (convert.allocatedSize == 0) return;

    uint32_t num_quads = convert.size;

    // set up the layout data in a form digestible by the GPU.
    if (!layoutValue)
    {
        layoutValue = TextLayoutValue::create();
        layoutValue->properties.dataVariance = DYNAMIC_DATA;
    }

    bool billboard = false;
    auto& layoutStruct = layoutValue->value();
    if (auto standardLayout = layout.cast<StandardLayout>(); standardLayout)
    {
        assignValue(layoutStruct.position, standardLayout->position, textLayoutUpdated);
        assignValue(layoutStruct.horizontal, standardLayout->horizontal, textLayoutUpdated);
        assignValue(layoutStruct.vertical, standardLayout->vertical, textLayoutUpdated);
        assignValue(layoutStruct.color, standardLayout->color, textLayoutUpdated);
        assignValue(layoutStruct.outlineColor, standardLayout->outlineColor, textLayoutUpdated);
        assignValue(layoutStruct.outlineWidth, standardLayout->outlineWidth, textLayoutUpdated);

        billboard = standardLayout->billboard;
        assignValue(layoutStruct.billboardAutoScaleDistance, standardLayout->billboardAutoScaleDistance, textLayoutUpdated);

        layoutValue->dirty();
    }

    // assign alignment offset
    auto alignment = layout->alignment(text->text, *(text->font));
    assignValue(layoutStruct.horizontalAlignment, alignment.x, textLayoutUpdated);
    assignValue(layoutStruct.verticalAlignment, alignment.y, textLayoutUpdated);

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

    ref_ptr<StateGroup> stateGroup = scenegraph.cast<StateGroup>();

    // create StateGroup as the root of the scene/command graph to hold the GraphicsPipeline, and binding of Descriptors to decorate the whole graph
    if (!stateGroup)
    {
        stateGroup = StateGroup::create();
        scenegraph = stateGroup;

        auto shaderSet = text->shaderSet ? text->shaderSet : createTextShaderSet(options);

        auto config = vsg::GraphicsPipelineConfigurator::create(shaderSet);

        auto& sharedObjects = text->font->sharedObjects;
        if (!sharedObjects) sharedObjects = SharedObjects::create();

        DataList arrays;
        config->assignArray(arrays, "inPosition", VK_VERTEX_INPUT_RATE_VERTEX, vertices);

        if (billboard)
        {
            config->shaderHints->defines.insert("BILLBOARD");
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

        auto glyphMetricSampler = Sampler::create();
        glyphMetricSampler->magFilter = VK_FILTER_NEAREST;
        glyphMetricSampler->minFilter = VK_FILTER_NEAREST;
        glyphMetricSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        glyphMetricSampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        glyphMetricSampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        glyphMetricSampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        glyphMetricSampler->unnormalizedCoordinates = VK_TRUE;

        if (sharedObjects) sharedObjects->share(glyphMetricSampler);

        uint32_t stride = sizeof(vec4);
        uint32_t numVec4PerGlyph = static_cast<uint32_t>(sizeof(GlyphMetrics) / sizeof(vec4));
        uint32_t numGlyphs = static_cast<uint32_t>(text->font->glyphMetrics->valueCount());

        auto glyphMetricsProxy = vec4Array2D::create(text->font->glyphMetrics, 0, stride, numVec4PerGlyph, numGlyphs, Data::Properties{VK_FORMAT_R32G32B32A32_SFLOAT});

        config->assignTexture("textureAtlas", text->font->atlas, sampler);
        config->assignTexture("glyphMetrics", glyphMetricsProxy, glyphMetricSampler);

        config->assignUniform("textLayout", layoutValue);
        config->assignUniform("text", textArray);

        // Set the InputAssemblyState.topology
        struct SetPipelineStates : public Visitor
        {
            void apply(Object& object) { object.traverse(*this); }
            void apply(InputAssemblyState& ias) { ias.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; }
        };
        vsg::visit<SetPipelineStates>(config);

        if (sharedObjects)
            sharedObjects->share(config, [](auto gpc) { gpc->init(); });
        else
            config->init();

        config->copyTo(stateGroup, sharedObjects);

        bindVertexBuffers = BindVertexBuffers::create(0, arrays);

        // setup geometry
        auto drawCommands = Commands::create();
        drawCommands->addChild(bindVertexBuffers);
        drawCommands->addChild(draw);
        stateGroup->addChild(drawCommands);
    }
    else
    {
    }
}
void GpuLayoutTechnique::setup(TextGroup* textGroup, uint32_t minimumAllocation, ref_ptr<const Options> options)
{
    info("GpuLayoutTechnique::setup(", textGroup, ", ", minimumAllocation, options, ") not yet supported");
}
