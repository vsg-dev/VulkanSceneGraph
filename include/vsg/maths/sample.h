#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/clamp.h>
#include <vsg/maths/color.h>
#include <vsg/state/Sampler.h>

namespace vsg
{
    /// return sample from an image equivalent to how the sample is computed on the GPU.
    template<class A>
    typename A::value_type sample(const Sampler& sampler, const A& image, vec2 coord)
    {
        using value_type = typename A::value_type;

        if (image.size() == 0 || !clamp(sampler.addressModeU, coord.x) || !clamp(sampler.addressModeV, coord.y))
        {
            return color_cast<value_type>(sampler.borderColor);
        }

        vec2 tc_scale(static_cast<float>(image.width()) - 1.0f, static_cast<float>(image.height()) - 1.0f);
        vec2 tc_index = coord * tc_scale;

        if (sampler.magFilter == VK_FILTER_NEAREST)
        {
            uint32_t i = static_cast<uint32_t>(tc_index.x + 0.5f);
            uint32_t j = static_cast<uint32_t>(tc_index.y + 0.5f);
            if (i >= image.width()) i = image.width() - 1;
            if (j >= image.height()) j = image.height() - 1;

            return image.at(i, j);
        }
        else // VK_FILTER_LINEAR
        {
            uint32_t i = static_cast<uint32_t>(tc_index.x);
            uint32_t j = static_cast<uint32_t>(tc_index.y);
            float r = tc_index.x - static_cast<float>(i);
            float s = tc_index.y - static_cast<float>(j);

            if (i >= image.width() - 1)
            {
                i = image.width() - 1;
                r = 0.0f;
            }

            if (j >= image.height() - 1)
            {
                j = image.height() - 1;
                s = 0.0f;
            }

            auto v = image.at(i, j) * ((1.0f - r) * (1.0f - s));
            if (r > 0.0) v += image.at(i + 1, j) * (r * (1.0f - s));
            if (s > 0.0)
            {
                v += image.at(i, j + 1) * ((1.0f - r) * s);
                if (r > 0.0) v += image.at(i + 1, j + 1) * (r * s);
            }

            return v;
        }
    }

} // namespace vsg
