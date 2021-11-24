#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/text/GlyphMetrics.h>

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
        ref_ptr<Options> options;

        /// get the index into the glyphMetrics array for the glyph associated with specified charcode
        uint32_t glyphIndexForCharcode(uint32_t charcode) const
        {
            if (charmap && charcode < charmap->size()) return charmap->at(charcode);
            return 0;
        }

        /// Wrapper for Font GPU state.
        struct VSG_DECLSPEC FontState : public Inherit<Object, FontState>
        {
            explicit FontState(Font* font);
            bool match() const { return true; }

            ref_ptr<DescriptorImage> textureAtlas;
            ref_ptr<DescriptorImage> glyphMetricsImage;
        };

        /// different text implementations may wish to share implementation details such as shaders etc.
        std::mutex sharedDataMutex;
        std::vector<ref_ptr<Object>> sharedData;

        /// get or create a Technique instance that matches the specified type
        template<class T, typename... Args>
        ref_ptr<T> getShared(Args&&... args)
        {
            {
                std::scoped_lock lock(sharedDataMutex);
                for (auto& shared : sharedData)
                {
                    auto required_data = shared.cast<T>();
                    if (required_data && required_data->match(args...)) return required_data;
                }
            }

            auto required_data = T::create(this, args...);
            {
                std::scoped_lock lock(sharedDataMutex);
                sharedData.emplace_back(required_data);
            }
            return required_data;
        }
    };
    VSG_type_name(vsg::Font);

} // namespace vsg
