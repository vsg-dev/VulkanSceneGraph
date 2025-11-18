#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/maths/vec4.h>

#include <vector>

namespace vsg
{

    class VSG_DECLSPEC MipmapLayout : public Inherit<Object, MipmapLayout>
    {
    public:

        MipmapLayout();

        explicit MipmapLayout(std::size_t size);

        using Mipmaps = std::vector<vsg::uivec4>;

        Mipmaps mipmaps;

        std::size_t size() const { return mipmaps.size(); }
        vsg::uivec4& at(size_t i) { return mipmaps[i]; }
        const vsg::uivec4& at(size_t i) const { return mipmaps[i]; }

        void set(size_t i, const vsg::uivec4& value) { mipmaps[i] = value; }

        Mipmaps::iterator begin() { return mipmaps.begin(); }
        Mipmaps::iterator end() { return mipmaps.end(); }

        Mipmaps::const_iterator begin() const { return mipmaps.begin(); }
        Mipmaps::const_iterator end() const { return mipmaps.end(); }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~MipmapLayout();
    };
    VSG_type_name(vsg::MipmapLayout);

} // namespace vsg
