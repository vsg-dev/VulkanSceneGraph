#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>

namespace vsg
{
    /// naming and meaning taken from FT_Glyph_Metrics, with dimensions normalized to fontHeight
    struct GlyphMetrics
    {
        float width;        // dimensions[0]
        float height;       // dimensions[1]
        float horiAdvance;  // dimensions[2]
        float vertAdvance;  // dimensions[3]
        float horiBearingX; // bearings[0]
        float horiBearingY; // bearings[1]
        float vertBearingX; // bearings[2]
        float vertBearingY; // bearings[3]
        vec4 uvrect;        // min x/y, max x/y

        void read(vsg::Input& input)
        {
            input.read("uvrect", uvrect);
            input.read("width", width);
            input.read("height", height);
            input.read("horiBearingX", horiBearingX);
            input.read("horiBearingY", horiBearingY);
            input.read("horiAdvance", horiAdvance);
            input.read("vertBearingX", vertBearingX);
            input.read("vertBearingY", vertBearingY);
            input.read("vertAdvance", vertAdvance);
        }

        void write(vsg::Output& output) const
        {
            output.write("uvrect", uvrect);
            output.write("width", width);
            output.write("height", height);
            output.write("horiBearingX", horiBearingX);
            output.write("horiBearingY", horiBearingY);
            output.write("horiAdvance", horiAdvance);
            output.write("vertBearingX", vertBearingX);
            output.write("vertBearingY", vertBearingY);
            output.write("vertAdvance", vertAdvance);
        }
    };

    template<>
    constexpr bool has_read_write<GlyphMetrics>() { return true; }

    VSG_array(GlyphMetricsArray, GlyphMetrics);

} // namespace vsg
