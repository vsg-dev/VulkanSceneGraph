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

class VSG_DECLSPEC GpuLayoutTechniqueArrayState : public Inherit<ArrayState, GpuLayoutTechniqueArrayState>
{
public:
    GpuLayoutTechniqueArrayState(const GpuLayoutTechnique* in_technique, const Text* in_text, bool in_billboard) :
        technique(in_technique),
        text(in_text),
        billboard(in_billboard)
    {
    }

    GpuLayoutTechniqueArrayState(const GpuLayoutTechniqueArrayState& rhs) :
        Inherit(rhs),
        technique(rhs.technique),
        text(rhs.text),
        billboard(rhs.billboard)
    {
    }

    explicit GpuLayoutTechniqueArrayState(const ArrayState& rhs) :
        Inherit(rhs)
    {
    }

    ref_ptr<ArrayState> cloneArrayState() override
    {
        return GpuLayoutTechniqueArrayState::create(*this);
    }

    ref_ptr<ArrayState> cloneArrayState(ref_ptr<ArrayState> arrayState) override
    {
        auto clone = GpuLayoutTechniqueArrayState::create(*arrayState);
        clone->technique = technique;
        clone->text = text;
        clone->billboard = billboard;
        return clone;
    }

    ref_ptr<const vec3Array> vertexArray(uint32_t instanceIndex) override
    {
        // compute the position of the glyph
        float horiAdvance = 0.0;
        float vertAdvance = 0.0;
        for (uint32_t i = 0; i < instanceIndex; ++i)
        {
            uint32_t glyph_index = technique->textArray->at(i);
            if (glyph_index == 0)
            {
                // treat as a newlline
                vertAdvance -= 1.0;
                horiAdvance = 0.0;
            }
            else
            {
                const GlyphMetrics& glyph_metrics = text->font->glyphMetrics->at(glyph_index);
                horiAdvance += glyph_metrics.horiAdvance;
            }
        }

        uint32_t glyph_index = technique->textArray->at(instanceIndex);
        const GlyphMetrics& glyph_metrics = text->font->glyphMetrics->at(glyph_index);

        // billboard effect
        auto textLayout = technique->layoutValue->value();
        dmat4 transform_to_local;
        if (billboard && !localToWorldStack.empty() && !worldToLocalStack.empty())
        {
            const dmat4& mv = localToWorldStack.back();
            const dmat4& inverse_mv = worldToLocalStack.back();
            dvec3 center_eye = mv * dvec3(textLayout.position);
            dmat4 billboard_mv = computeBillboardMatrix(center_eye, (double)textLayout.billboardAutoScaleDistance);
            transform_to_local = inverse_mv * billboard_mv;
        }
        else
        {
            transform_to_local = vsg::translate(textLayout.position);
        }

        auto new_vertices = vsg::vec3Array::create(6);
        auto src_vertex_itr = vertices->begin();
        for (auto& v : *new_vertices)
        {
            const auto& sv = *(src_vertex_itr++);

            // compute the position of vertex
            vec3 pos = textLayout.horizontal * (horiAdvance + textLayout.horizontalAlignment + glyph_metrics.horiBearingX + sv.x * glyph_metrics.width) +
                       textLayout.vertical * (vertAdvance + textLayout.verticalAlignment + glyph_metrics.horiBearingY + (sv.y - 1.f) * glyph_metrics.height);

            v = vec3(transform_to_local * dvec3(pos));
        }

        return new_vertices;
    }

    const GpuLayoutTechnique* technique = nullptr;
    const Text* text = nullptr;
    bool billboard = false;
};

template<typename T>
void assignValue(T& dest, const T& src, bool& updated)
{
    if (dest == src) return;
    dest = src;
    updated = true;
}

void GpuLayoutTechnique::setup(Text* text, uint32_t minimumAllocation, ref_ptr<const Options> options)
{
    if (!text || !(text->text) || !text->font || !text->layout) return;

    auto& layout = text->layout;

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
                assignValue(*(itr++), font.glyphIndexForCharcode(uint32_t(c)), updated);
            }
        }
        void apply(const wstringValue& text) override
        {
            allocate(static_cast<uint32_t>(text.value().size()));

            auto itr = textArray->begin();
            for (auto& c : text.value())
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(uint32_t(c)), updated);
            }
        }
        void apply(const ubyteArray& text) override
        {
            allocate(static_cast<uint32_t>(text.valueCount()));

            auto itr = textArray->begin();
            for (const auto& c : text)
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(c), updated);
            }
        }
        void apply(const ushortArray& text) override
        {
            allocate(static_cast<uint32_t>(text.valueCount()));

            auto itr = textArray->begin();
            for (const auto& c : text)
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(c), updated);
            }
        }
        void apply(const uintArray& text) override
        {
            allocate(static_cast<uint32_t>(text.valueCount()));

            auto itr = textArray->begin();
            for (const auto& c : text)
            {
                assignValue(*(itr++), font.glyphIndexForCharcode(c), updated);
            }
        }
    };

    ConvertString converter(*(text->font), textArray, textArrayUpdated, minimumAllocation);
    text->text->accept(converter);

    if (converter.allocatedSize == 0) return;

    uint32_t num_quads = converter.size;

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
        vertices = vec3Array::create(6);

        float leadingEdgeGradient = 0.1f;

        vertices->set(0, vec3(0.0f, 1.0f, 2.0f * leadingEdgeGradient));
        vertices->set(1, vec3(0.0f, 0.0f, leadingEdgeGradient));
        vertices->set(2, vec3(1.0f, 1.0f, leadingEdgeGradient));

        vertices->set(3, vec3(0.0f, 0.0f, leadingEdgeGradient));
        vertices->set(4, vec3(1.0f, 0.0f, 0.0f));
        vertices->set(5, vec3(1.0f, 1.0f, leadingEdgeGradient));
    }

    if (!draw)
        draw = Draw::create(6, num_quads, 0, 0);
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
        if (!text->font->atlasImageInfo)
        {
            text->font->createFontImages();
        }
        config->assignTexture("textureAtlas", {text->font->atlasImageInfo}, 0);
        config->assignTexture("glyphMetrics", {text->font->glyphImageInfo}, 0);

        config->assignDescriptor("textLayout", layoutValue);
        config->assignDescriptor("text", textArray);

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

        // Assign ArrayState for CPU mapping of vertices
        stateGroup->prototypeArrayState = GpuLayoutTechniqueArrayState::create(this, text, billboard);
    }
    else
    {
    }
}
void GpuLayoutTechnique::setup(TextGroup* textGroup, uint32_t minimumAllocation, ref_ptr<const Options> options)
{
    info("GpuLayoutTechnique::setup(", textGroup, ", ", minimumAllocation, options, ") not yet supported");
}
