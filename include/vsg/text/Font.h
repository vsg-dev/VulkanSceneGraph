#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/io/Options.h>
#include <vsg/text/GlyphMetrics.h>
#include <vsg/utils/SharedObjects.h>

namespace vsg
{
    class VSG_DECLSPEC Font : public Inherit<Object, Font>
    {
    public:
        Font();

        void read(Input& input) override;
        void write(Output& output) const override;

        float ascender = 1.0f;  // maximum ascent below the baseline
        float descender = 0.0f; // maximum descent below the baseline
        float height = 1.0f;    // vertical distance between two consecutive baselines

        ref_ptr<Data> atlas;
        ref_ptr<GlyphMetricsArray> glyphMetrics;
        ref_ptr<uintArray> charmap;
        ref_ptr<SharedObjects> sharedObjects;

        /// get the index into the glyphMetrics array for the glyph associated with specified charcode
        uint32_t glyphIndexForCharcode(uint32_t charcode) const
        {
            if (charmap && charcode < charmap->size()) return charmap->at(charcode);
            return 0;
        }
    };
    VSG_type_name(vsg::Font);

} // namespace vsg
