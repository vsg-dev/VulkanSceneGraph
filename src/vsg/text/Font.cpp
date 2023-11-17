/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/text/TextLayout.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

Font::Font()
{
}

void Font::read(Input& input)
{
    Object::read(input);

    input.read("ascender", ascender);
    input.read("descender", descender);
    input.read("height", height);

    input.readObject("charmap", charmap);
    input.readObject("glyphMetrics", glyphMetrics);
    input.readObject("atlas", atlas);

    if (input.version_less(0, 5, 5))
    {
        ref_ptr<Options> options;
        input.readObject("options", options);
    }
}

void Font::write(Output& output) const
{
    Object::write(output);

    output.write("ascender", ascender);
    output.write("descender", descender);
    output.write("height", height);

    output.writeObject("charmap", charmap);
    output.writeObject("glyphMetrics", glyphMetrics);
    output.writeObject("atlas", atlas);

    if (output.version_less(0, 5, 5))
    {
        ref_ptr<Options> options;
        output.writeObject("options", options);
    }
}

void Font::createFontImages()
{
    if (!atlasImageInfo)
    {
        auto sampler = Sampler::create();
        sampler->magFilter = VK_FILTER_LINEAR;
        sampler->minFilter = VK_FILTER_LINEAR;
        sampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler->borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        sampler->anisotropyEnable = VK_TRUE;
        sampler->maxAnisotropy = 16.0f;
        sampler->maxLod = 12.0;
        atlasImageInfo = ImageInfo::create(sampler, atlas);
    }
    if (!glyphImageInfo)
    {
        auto glyphMetricSampler = Sampler::create();
        glyphMetricSampler->magFilter = VK_FILTER_NEAREST;
        glyphMetricSampler->minFilter = VK_FILTER_NEAREST;
        glyphMetricSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        glyphMetricSampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        glyphMetricSampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        glyphMetricSampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        glyphMetricSampler->unnormalizedCoordinates = VK_TRUE;

        uint32_t stride = sizeof(vec4);
        uint32_t numVec4PerGlyph = static_cast<uint32_t>(sizeof(GlyphMetrics) / sizeof(vec4));
        uint32_t numGlyphs = static_cast<uint32_t>(glyphMetrics->valueCount());

        auto glyphMetricsProxy = vec4Array2D::create(glyphMetrics, 0, stride, numVec4PerGlyph, numGlyphs,
                                                     Data::Properties{VK_FORMAT_R32G32B32A32_SFLOAT});
        glyphImageInfo = ImageInfo::create(glyphMetricSampler, glyphMetricsProxy);
    }
}
