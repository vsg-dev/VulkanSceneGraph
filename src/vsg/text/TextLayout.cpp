/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/text/TextLayout.h>

#include <iostream>

using namespace vsg;

void LeftAlignment::read(Input& input)
{
    TextLayout::read(input);

    input.read("position", position);
    input.read("horizontal", horizontal);
    input.read("vertical", vertical);
    input.read("color", color);
}

void LeftAlignment::write(Output& output) const
{
    TextLayout::write(output);

    output.write("position", position);
    output.write("horizontal", horizontal);
    output.write("vertical", vertical);
    output.write("color", color);
}

void LeftAlignment::layout(const Data* text, const Font& font, TextQuads& quads)
{
    quads.clear();

    struct Convert : public ConstVisitor
    {
        const LeftAlignment& layout;
        const Font& font;
        TextQuads& textQuads;

        vec3 row_position;
        vec3 pen_position;
        vec3 normal;

        Convert(const LeftAlignment& in_layout, const Font& in_font, TextQuads& in_textQuads) :
            layout(in_layout),
            font(in_font),
            textQuads(in_textQuads)
        {
            row_position = layout.position;
            pen_position = row_position;
            normal = normalize(cross(layout.horizontal, layout.vertical));
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

        void character(uint32_t charcode)
        {
            if (charcode == '\n')
            {
                // newline
                row_position -= layout.vertical;
                pen_position = row_position;
            }
            else if (charcode == ' ')
            {
                // space
                if (auto itr = font.glyphs.find(charcode); itr != font.glyphs.end())
                {
                    const auto& glyph = itr->second;
                    pen_position += layout.horizontal * glyph.horiAdvance;
                }
                else
                {
                    pen_position += layout.horizontal;
                }
            }
            else
            {
                TextQuad quad;

                auto itr = font.glyphs.find(charcode);
                if (itr == font.glyphs.end()) return;

                const auto& glyph = itr->second;
                const auto& uvrect = glyph.uvrect;

                vec3 local_origin = pen_position + layout.horizontal * glyph.horiBearingX + layout.vertical * glyph.horiBearingY - layout.vertical * glyph.height;

                quad.vertices[0] = local_origin;
                quad.vertices[1] = local_origin + layout.horizontal * glyph.width;
                quad.vertices[2] = local_origin + layout.horizontal * glyph.width + layout.vertical * glyph.height;
                quad.vertices[3] = local_origin + layout.vertical * glyph.height;

                quad.colors[0] = layout.color;
                quad.colors[1] = layout.color;
                quad.colors[2] = layout.color;
                quad.colors[3] = layout.color;

                quad.texcoords[0].set(uvrect[0], uvrect[1]);
                quad.texcoords[1].set(uvrect[2], uvrect[1]);
                quad.texcoords[2].set(uvrect[2], uvrect[3]);
                quad.texcoords[3].set(uvrect[0], uvrect[3]);

                quad.normal = normal;

                textQuads.push_back(quad);

                pen_position += layout.horizontal * glyph.horiAdvance;
            }
        }
    };

    Convert converter(*this, font, quads);

    text->accept(converter);
}
