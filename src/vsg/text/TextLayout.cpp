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

void LeftAlignment::layout(const std::string& text, const Font& font, TextQuads& quads)
{
    quads.clear();
    quads.reserve(text.size());

    vec3 row_position = position;
    vec3 pen_position = row_position;
    vec3 normal = normalize(cross(horizontal, vertical));
    for(auto& character : text)
    {
        if (character == '\n')
        {
            // newline
            row_position -= vertical;
            pen_position = row_position;
        }
        else if (character == ' ')
        {
            // space
            uint16_t charcode(character);
            if (auto itr = font.glyphs.find(charcode); itr != font.glyphs.end())
            {
                const auto& glyph = itr->second;
                pen_position += horizontal * glyph.horiAdvance;
            }
            else
            {
                pen_position += horizontal;
            }
        }
        else
        {
            TextQuad quad;

            uint16_t charcode(character);
            auto itr = font.glyphs.find(charcode);
            if (itr == font.glyphs.end()) continue;

            const auto& glyph = itr->second;
            const auto& uvrect = glyph.uvrect;

            vec3 local_origin = pen_position + horizontal * glyph.horiBearingX + vertical * glyph.horiBearingY;

            quad.vertices[0] = local_origin;
            quad.vertices[1] = local_origin + horizontal * glyph.width;
            quad.vertices[2] = local_origin + horizontal * glyph.width + vertical * glyph.height;
            quad.vertices[3] = local_origin + vertical * glyph.height;

            quad.colors[0] = color;
            quad.colors[1] = color;
            quad.colors[2] = color;
            quad.colors[3] = color;

            quad.texcoords[0].set(uvrect[0], uvrect[1]);
            quad.texcoords[1].set(uvrect[0]+uvrect[2], uvrect[1]);
            quad.texcoords[2].set(uvrect[0]+uvrect[2], uvrect[1]+uvrect[3]);
            quad.texcoords[3].set(uvrect[0], uvrect[1]+uvrect[3]);

            quad.normal = normal;

            quads.push_back(quad);

            pen_position += horizontal * glyph.horiAdvance;
        }
    }
}
