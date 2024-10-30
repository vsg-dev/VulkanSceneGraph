/* <editor-fold desc="MIT License">

Copyright(c) 2024 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/utils/ColorSpaceConvertor.h>

using namespace vsg;

ColorSpaceConvertor::ColorSpaceConvertor()
{
}

void ColorSpaceConvertor::convertVertexColors(vec4Array& colors) const
{
    for (auto& color : colors)
        convertVertexColor(color);
}

void ColorSpaceConvertor::convertVertexColors(dvec4Array& colors) const
{
    for (auto& color : colors)
        convertVertexColor(color);
}

void ColorSpaceConvertor::convertMaterialColors(vec4Array& colors) const
{
    for (auto& color : colors)
        convertMaterialColor(color);
}

void ColorSpaceConvertor::convertMaterialColors(dvec4Array& colors) const
{
    for (auto& color : colors)
        convertMaterialColor(color);
}

void sRGB_to_linearColorSpaceConvertor::convertVertexColors(vec4Array& colors) const
{
    for (auto& color : colors)
        color = sRGB_to_linear(color);
}

void sRGB_to_linearColorSpaceConvertor::convertVertexColors(dvec4Array& colors) const
{
    for (auto& color : colors)
        color = sRGB_to_linear(color);
}

void sRGB_to_linearColorSpaceConvertor::convertMaterialColors(vec4Array& colors) const
{
    for (auto& color : colors)
        color = sRGB_to_linear(color);
}

void sRGB_to_linearColorSpaceConvertor::convertMaterialColors(dvec4Array& colors) const
{
    for (auto& color : colors)
        color = sRGB_to_linear(color);
}

void linear_to_sRGBColorSpaceConvertor::convertVertexColors(vec4Array& colors) const
{
    for (auto& color : colors)
        color = linear_to_sRGB(color);
}

void linear_to_sRGBColorSpaceConvertor::convertVertexColors(dvec4Array& colors) const
{
    for (auto& color : colors)
        color = linear_to_sRGB(color);
}

void linear_to_sRGBColorSpaceConvertor::convertMaterialColors(vec4Array& colors) const
{
    for (auto& color : colors)
        color = linear_to_sRGB(color);
}

void linear_to_sRGBColorSpaceConvertor::convertMaterialColors(dvec4Array& colors) const
{
    for (auto& color : colors)
        color = linear_to_sRGB(color);
}
