#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/io/Options.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/DescriptorSet.h>

namespace vsg
{
    class VSG_DECLSPEC Font : public Inherit<Object, Font>
    {
    public:

        Font();

        struct GlyphData
        {
            uint16_t character;
            vec4 uvrect; // min x/y, max x/y
            vec2 size; // normalised size of the glyph
            vec2 offset; // normalised offset
            float xadvance; // normalised xadvance
            float lookupOffset; // offset into lookup texture
        };
        using GlyphMap = std::map<uint16_t, GlyphData>;

        ref_ptr<Data> atlas;
        GlyphMap glyphs;
        float fontHeight;
        float normalisedLineHeight;
        ref_ptr<Options> options;

        /// different text impplementations may wish to share implementation details such as shaders etc.
        std::vector<ref_ptr<Object>> sharedData;

        /// get or create a Technique instance that matches the specified type
        template<class T>
        ref_ptr<T> getShared()
        {
            for(auto& shared : sharedData)
            {
                auto required_data = shared.cast<T>();
                if (required_data) return required_data;
            }
            auto required_data = T::create(this);
            sharedData.emplace_back(required_data);
            return required_data;
        }
    };
}
