/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/text/TextLayout.h>

using namespace vsg;

Font::Font()
{
}

void Font::read(Input& input)
{
    Object::read(input);

    input.read("fontHeight", fontHeight);
    input.read("normalisedLineHeight", normalisedLineHeight);
    input.readObject("atlas", atlas);

    glyphs.clear();

    uint32_t numGlyphs = input.readValue<uint32_t>("numGlyphs");
    for (uint32_t i = 0; i < numGlyphs; ++i)
    {
        uint16_t charcode;
        input.read("charcode", charcode);

        GlyphMetrics& glyph = glyphs[charcode];
        glyph.charcode = charcode;

        input.read("uvrect", glyph.uvrect);
        input.read("width", glyph.width);
        input.read("height", glyph.height);
        input.read("horiBearingX", glyph.horiBearingX);
        input.read("horiBearingY", glyph.horiBearingY);
        input.read("horiAdvance", glyph.horiAdvance);
        input.read("vertBearingX", glyph.vertBearingX);
        input.read("vertBearingY", glyph.vertBearingY);
        input.read("vertAdvance", glyph.vertAdvance);
    }
}

void Font::write(Output& output) const
{
    Object::write(output);

    output.write("fontHeight", fontHeight);
    output.write("normalisedLineHeight", normalisedLineHeight);
    output.writeObject("atlas", atlas);

    output.writeValue<uint32_t>("numGlyphs", glyphs.size());
    for (auto itr = glyphs.begin(); itr != glyphs.end(); ++itr)
    {
        const GlyphMetrics& glyph = itr->second;
        output.write("charcode", glyph.charcode);

        output.write("uvrect", glyph.uvrect);
        output.write("width", glyph.width);
        output.write("height", glyph.height);
        output.write("horiBearingX", glyph.horiBearingX);
        output.write("horiBearingY", glyph.horiBearingY);
        output.write("horiAdvance", glyph.horiAdvance);
        output.write("vertBearingX", glyph.vertBearingX);
        output.write("vertBearingY", glyph.vertBearingY);
        output.write("vertAdvance", glyph.vertAdvance);
    }
}
