#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/core/Inherit.h>
#include <vsg/maths/color.h>

namespace vsg
{
    /// ColorSpaceConvertor is a base class for switching colors in the scene graph between color spaces.
    class VSG_DECLSPEC ColorSpaceConvertor : public Inherit<Object, ColorSpaceConvertor>
    {
    public:

        ColorSpaceConvertor();

        //
        // provide virtual functions for concrete BaseColorSpaceConvertor implementations to actually convert colors
        // default implementations are provided for the array overloads, but implementations may be faster if they implement them anyway so the conversion can be inlined
        virtual void convertVertexColor(vec4& color) const = 0;
        virtual void convertVertexColor(dvec4& color) const = 0;
        virtual void convertVertexColors(vec4Array& colors) const;
        virtual void convertVertexColors(dvec4Array& colors) const;

        virtual void convertMaterialColor(vec4& color) const = 0;
        virtual void convertMaterialColor(dvec4& color) const = 0;
        virtual void convertMaterialColors(vec4Array& colors) const;
        virtual void convertMaterialColors(dvec4Array& colors) const;
    };
    VSG_type_name(vsg::ColorSpaceConvertor);

    class VSG_DECLSPEC NoOpColorSpaceConvertor : public Inherit<ColorSpaceConvertor, NoOpColorSpaceConvertor>
    {
    public:
        void convertVertexColor(vec4& color) const override{};
        void convertVertexColor(dvec4& color) const override{};
        void convertVertexColors(vec4Array& colors) const override{};
        void convertVertexColors(dvec4Array& colors) const override{};

        void convertMaterialColor(vec4& color) const override{};
        void convertMaterialColor(dvec4& color) const override{};
        void convertMaterialColors(vec4Array& colors) const override{};
        void convertMaterialColors(dvec4Array& colors) const override{};
    };
    VSG_type_name(vsg::NoOpColorSpaceConvertor);

    class VSG_DECLSPEC sRGB_to_linearColorSpaceConvertor : public Inherit<ColorSpaceConvertor, sRGB_to_linearColorSpaceConvertor>
    {
    public:
        void convertVertexColor(vec4& color) const override { color = sRGB_to_linear(color); };
        void convertVertexColor(dvec4& color) const override { color = sRGB_to_linear(color); };
        void convertVertexColors(vec4Array& colors) const override;
        void convertVertexColors(dvec4Array& colors) const override;

        void convertMaterialColor(vec4& color) const override { color = sRGB_to_linear(color); };
        void convertMaterialColor(dvec4& color) const override { color = sRGB_to_linear(color); };
        void convertMaterialColors(vec4Array& colors) const override;
        void convertMaterialColors(dvec4Array& colors) const override;
    };
    VSG_type_name(vsg::sRGB_to_linearColorSpaceConvertor);

    class VSG_DECLSPEC linear_to_sRGBColorSpaceConvertor : public Inherit<ColorSpaceConvertor, linear_to_sRGBColorSpaceConvertor>
    {
    public:
        void convertVertexColor(vec4& color) const override { color = linear_to_sRGB(color); };
        void convertVertexColor(dvec4& color) const override { color = linear_to_sRGB(color); };
        void convertVertexColors(vec4Array& colors) const override;
        void convertVertexColors(dvec4Array& colors) const override;

        void convertMaterialColor(vec4& color) const override { color = linear_to_sRGB(color); };
        void convertMaterialColor(dvec4& color) const override { color = linear_to_sRGB(color); };
        void convertMaterialColors(vec4Array& colors) const override;
        void convertMaterialColors(dvec4Array& colors) const override;
    };
    VSG_type_name(vsg::linear_to_sRGBColorSpaceConvertor);
} // namespace vsg
