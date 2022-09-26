#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/maths/box.h>
#include <vsg/text/Font.h>

namespace vsg
{
    struct TextQuad
    {
        vec3 vertices[4];
        vec2 texcoords[4];
        vec4 colors[4];
        vec4 outlineColors[4];
        float outlineWidths[4];
        vec3 normal;
        vec4 centerAndAutoScaleDistance; // only used by when billboarding
    };

    using TextQuads = std::vector<TextQuad>;

    class VSG_DECLSPEC TextLayout : public Inherit<Object, TextLayout>
    {
    public:
        virtual bool requiresBillboard() const { return false; }
        virtual void layout(const Data* text, const Font& font, TextQuads& texQuads) = 0;
        virtual vec2 alignment(const Data* text, const Font& font) const = 0;
        virtual dbox extents(const Data* text, const Font& font) const = 0;
    };
    VSG_type_name(vsg::TextLayout);

} // namespace vsg
