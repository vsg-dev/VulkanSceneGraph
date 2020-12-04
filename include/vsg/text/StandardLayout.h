#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/text/TextLayout.h>

namespace vsg
{

    class VSG_DECLSPEC StandardLayout : public Inherit<TextLayout, StandardLayout>
    {
    public:
        void read(Input& input) override;
        void write(Output& output) const override;

        enum Alignment
        {
            BASELINE_ALIGNMENT,
            LEFT_ALIGNMENT,
            TOP_ALIGNMENT = LEFT_ALIGNMENT,
            CENTER_ALIGNMENT,
            RIGHT_ALIGNMENT,
            BOTTOM_ALIGNMENT = RIGHT_ALIGNMENT
        };

        enum GlyphLayout
        {
            LEFT_TO_RIGHT_LAYOUT,
            RIGHT_TO_LEFT_LAYOUT,
            VERTICAL_LAYOUT
        };

        Alignment horizontalAlignment = BASELINE_ALIGNMENT;
        Alignment verticalAlignment = BASELINE_ALIGNMENT;
        GlyphLayout glyphLayout = LEFT_TO_RIGHT_LAYOUT;
        vec3 position = vec3(0.0f, 0.0f, 0.0f);
        vec3 horizontal = vec3(1.0f, 0.0f, 0.0f);
        vec3 vertical = vec3(0.0f, 1.0f, 0.0f);
        vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        vec4 outlineColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float outlineWidth = 0.0f;

        void layout(const Data* text, const Font& font, TextQuads& texQuads) override;
    };
    VSG_type_name(vsg::StandardLayout);

} // namespace vsg
