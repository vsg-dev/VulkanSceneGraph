/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/text/StandardLayout.h>

using namespace vsg;

void StandardLayout::read(Input& input)
{
    TextLayout::read(input);

    input.readValue<uint32_t>("horizontalAlignment", horizontalAlignment);
    input.readValue<uint32_t>("verticalAlignment", verticalAlignment);
    input.readValue<uint32_t>("glyphLayout", glyphLayout);
    input.read("position", position);
    input.read("horizontal", horizontal);
    input.read("vertical", vertical);
    input.read("color", color);
    input.read("outlineColor", outlineColor);
    input.read("outlineWidth", outlineWidth);
}

void StandardLayout::write(Output& output) const
{
    TextLayout::write(output);

    output.writeValue<uint32_t>("horizontalAlignment", horizontalAlignment);
    output.writeValue<uint32_t>("verticalAlignment", verticalAlignment);
    output.writeValue<uint32_t>("glyphLayout", glyphLayout);
    output.write("position", position);
    output.write("horizontal", horizontal);
    output.write("vertical", vertical);
    output.write("color", color);
    output.write("outlineColor", outlineColor);
    output.write("outlineWidth", outlineWidth);
}

void StandardLayout::layout(const Data* text, const Font& font, TextQuads& quads)
{
    quads.clear();

    struct Convert : public ConstVisitor
    {
        const StandardLayout& layout;
        const Font& font;
        TextQuads& textQuads;
        size_t start_of_row;

        vec3 row_position;
        vec3 pen_position;
        vec3 normal;

        Convert(const StandardLayout& in_layout, const Font& in_font, TextQuads& in_textQuads) :
            layout(in_layout),
            font(in_font),
            textQuads(in_textQuads)
        {
            row_position.set(0.0f, 0.0f, 0.0f);
            pen_position = row_position;
            normal = normalize(cross(layout.horizontal, layout.vertical));
            start_of_row = textQuads.size();
        }

        void apply(const stringValue& text) override
        {
            reserve(text.value().size());
            for (auto& c : text.value())
            {
                character(uint8_t(c));
            }
        }
        void apply(const ubyteArray& text) override
        {
            reserve(text.size());
            for (auto& c : text)
            {
                character(c);
            }
        }
        void apply(const ushortArray& text) override
        {
            reserve(text.size());
            for (auto& c : text)
            {
                character(c);
            }
        }
        void apply(const uintArray& text) override
        {
            reserve(text.size());
            for (auto& c : text)
            {
                character(c);
            }
        }

        void reserve(size_t size)
        {
            textQuads.reserve(size);
        }

        void translate(TextQuads::iterator itr, TextQuads::iterator end, const vec3& offset)
        {
            for (; itr != end; ++itr)
            {
                TextQuad& quad = *itr;
                quad.vertices[0] += offset;
                quad.vertices[1] += offset;
                quad.vertices[2] += offset;
                quad.vertices[3] += offset;
            }
        }

        void align_row()
        {
            if (start_of_row >= textQuads.size()) return;

            if (layout.glyphLayout == VERTICAL_LAYOUT)
            {
                if (layout.verticalAlignment == StandardLayout::BASELINE_ALIGNMENT) return;
            }
            else if (layout.horizontalAlignment == StandardLayout::BASELINE_ALIGNMENT)
                return;

            switch (layout.glyphLayout)
            {
            case (LEFT_TO_RIGHT_LAYOUT):
            case (RIGHT_TO_LEFT_LAYOUT): {
                float left = textQuads[start_of_row].vertices[0].x;
                float right = textQuads[start_of_row].vertices[1].x;
                for (size_t i = start_of_row + 1; i < textQuads.size(); ++i)
                {
                    if (textQuads[i].vertices[0].x < left) left = textQuads[i].vertices[0].x;
                    if (textQuads[i].vertices[1].x > right) right = textQuads[i].vertices[1].x;
                }

                float target = left;
                switch (layout.horizontalAlignment)
                {
                case (BASELINE_ALIGNMENT):
                case (LEFT_ALIGNMENT): target = left; break;
                case (CENTER_ALIGNMENT): target = (right + left) * 0.5f; break;
                case (RIGHT_ALIGNMENT): target = right; break;
                }

                vec3 offset(-(target - left), 0.0f, 0.0f);
                translate(textQuads.begin() + start_of_row, textQuads.end(), offset);
                break;
            }
            case (VERTICAL_LAYOUT): {
                float bottom = textQuads[start_of_row].vertices[0].y;
                float top = textQuads[start_of_row].vertices[3].y;
                for (size_t i = start_of_row + 1; i < textQuads.size(); ++i)
                {
                    if (textQuads[i].vertices[0].y < bottom) bottom = textQuads[i].vertices[0].y;
                    if (textQuads[i].vertices[3].y > top) top = textQuads[i].vertices[3].y;
                }
                float target = top;
                switch (layout.verticalAlignment)
                {
                case (BASELINE_ALIGNMENT):
                case (TOP_ALIGNMENT): target = top; break;
                case (CENTER_ALIGNMENT): target = (top + bottom) * 0.5f; break;
                case (BOTTOM_ALIGNMENT): target = bottom; break;
                }

                vec3 offset(0.0f, -(target - top), 0.0f);
                translate(textQuads.begin() + start_of_row, textQuads.end(), offset);
                break;
            }
            }
        }

        void finalize()
        {
            align_row();

            vec3 offset(0.0f, 0.0f, 0.0f);

            if (layout.horizontalAlignment != BASELINE_ALIGNMENT || layout.verticalAlignment != BASELINE_ALIGNMENT)
            {
                float left = textQuads[start_of_row].vertices[0].x;
                float right = textQuads[start_of_row].vertices[1].x;
                float bottom = textQuads[start_of_row].vertices[0].y;
                float top = textQuads[start_of_row].vertices[3].y;
                for (size_t i = 0; i < textQuads.size(); ++i)
                {
                    if (textQuads[i].vertices[0].x < left) left = textQuads[i].vertices[0].x;
                    if (textQuads[i].vertices[1].x > right) right = textQuads[i].vertices[1].x;
                    if (textQuads[i].vertices[0].y < bottom) bottom = textQuads[i].vertices[0].y;
                    if (textQuads[i].vertices[3].y > top) top = textQuads[i].vertices[3].y;
                }

                switch (layout.horizontalAlignment)
                {
                case (BASELINE_ALIGNMENT): offset.x = 0.0f; break;
                case (LEFT_ALIGNMENT): offset.x = -left; break;
                case (CENTER_ALIGNMENT): offset.x = -(right + left) * 0.5f; break;
                case (RIGHT_ALIGNMENT): offset.x = -right; break;
                }

                switch (layout.verticalAlignment)
                {
                case (BASELINE_ALIGNMENT): offset.y = 0.0f; break;
                case (TOP_ALIGNMENT): offset.y = -top; break;
                case (CENTER_ALIGNMENT): offset.y = -(bottom + top) * 0.5f; break;
                case (BOTTOM_ALIGNMENT): offset.y = -bottom; break;
                }
            }

            if (offset.x != 0.0f || offset.y != 0.0f) translate(textQuads.begin(), textQuads.end(), offset);

            for (auto& quad : textQuads)
            {
                quad.vertices[0] = layout.position + layout.horizontal * quad.vertices[0].x + layout.vertical * quad.vertices[0].y;
                quad.vertices[1] = layout.position + layout.horizontal * quad.vertices[1].x + layout.vertical * quad.vertices[1].y;
                quad.vertices[2] = layout.position + layout.horizontal * quad.vertices[2].x + layout.vertical * quad.vertices[2].y;
                quad.vertices[3] = layout.position + layout.horizontal * quad.vertices[3].x + layout.vertical * quad.vertices[3].y;
            }
        }

        void character(uint32_t charcode)
        {
            if (charcode == '\n')
            {
                align_row();

                // newline
                switch (layout.glyphLayout)
                {
                case (LEFT_TO_RIGHT_LAYOUT):
                case (RIGHT_TO_LEFT_LAYOUT):
                    row_position.y -= 1.0f;
                    break;
                case (VERTICAL_LAYOUT):
                    row_position.x += 1.0f;
                    break;
                }
                pen_position = row_position;
                start_of_row = textQuads.size();
            }
            else if (charcode == ' ')
            {
                // space
                if (auto glyph_index = font.glyphIndexForCharcode(charcode))
                {
                    const auto& glyph = (*font.glyphMetrics)[glyph_index];

                    switch (layout.glyphLayout)
                    {
                    case (LEFT_TO_RIGHT_LAYOUT):
                        pen_position.x += glyph.horiAdvance;
                        break;
                    case (RIGHT_TO_LEFT_LAYOUT):
                        pen_position.x -= glyph.horiAdvance;
                        break;
                    case (VERTICAL_LAYOUT):
                        pen_position.y -= glyph.vertAdvance;
                        break;
                    }
                }
                else
                {
                    switch (layout.glyphLayout)
                    {
                    case (LEFT_TO_RIGHT_LAYOUT):
                        pen_position.x += 1.0f;
                        break;
                    case (RIGHT_TO_LEFT_LAYOUT):
                        pen_position.x -= 1.0f;
                        break;
                    case (VERTICAL_LAYOUT):
                        pen_position.y -= 1.0f;
                        break;
                    }
                }
            }
            else
            {
                auto glyph_index = font.glyphIndexForCharcode(charcode);
                if (glyph_index == 0) return;

                const auto& glyph = (*font.glyphMetrics)[glyph_index];
                const auto& uvrect = glyph.uvrect;

                vec3 local_origin = pen_position;
                switch (layout.glyphLayout)
                {
                case (LEFT_TO_RIGHT_LAYOUT):
                    local_origin += vec3(glyph.horiBearingX, glyph.horiBearingY - glyph.height, 0.0f);
                    break;
                case (RIGHT_TO_LEFT_LAYOUT):
                    local_origin += vec3(-glyph.width + glyph.horiBearingX, glyph.horiBearingY - glyph.height, 0.0f);
                    break;
                case (VERTICAL_LAYOUT):
                    local_origin += vec3(glyph.vertBearingX, glyph.vertBearingY - glyph.height, 0.0f);
                    break;
                }

                TextQuad quad;

                quad.vertices[0] = local_origin;
                quad.vertices[1] = local_origin + vec3(glyph.width, 0.0f, 0.0f);
                quad.vertices[2] = local_origin + vec3(glyph.width, glyph.height, 0.0f);
                quad.vertices[3] = local_origin + vec3(0.0f, glyph.height, 0.0f);

                quad.colors[0] = layout.color;
                quad.colors[1] = layout.color;
                quad.colors[2] = layout.color;
                quad.colors[3] = layout.color;

                quad.texcoords[0].set(uvrect[0], uvrect[1]);
                quad.texcoords[1].set(uvrect[2], uvrect[1]);
                quad.texcoords[2].set(uvrect[2], uvrect[3]);
                quad.texcoords[3].set(uvrect[0], uvrect[3]);

                quad.outlineColors[0] = layout.outlineColor;
                quad.outlineColors[1] = layout.outlineColor;
                quad.outlineColors[2] = layout.outlineColor;
                quad.outlineColors[3] = layout.outlineColor;

                quad.outlineWidths[0] = layout.outlineWidth;
                quad.outlineWidths[1] = layout.outlineWidth;
                quad.outlineWidths[2] = layout.outlineWidth;
                quad.outlineWidths[3] = layout.outlineWidth;

                quad.normal = normal;

                textQuads.push_back(quad);

                switch (layout.glyphLayout)
                {
                case (LEFT_TO_RIGHT_LAYOUT):
                    pen_position.x += glyph.horiAdvance;
                    break;
                case (RIGHT_TO_LEFT_LAYOUT):
                    pen_position.x -= glyph.horiAdvance;
                    break;
                case (VERTICAL_LAYOUT):
                    pen_position.y -= glyph.vertAdvance;
                    break;
                }
            }
        }
    };

    Convert converter(*this, font, quads);

    text->accept(converter);

    converter.finalize();
}
