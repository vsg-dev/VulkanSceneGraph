/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/lighting/SoftShadows.h>

using namespace vsg;

SoftShadows::SoftShadows(uint32_t in_shadowMaps, float in_penumbraRadius) :
    Inherit(in_shadowMaps),
    penumbraRadius(in_penumbraRadius)
{
}

SoftShadows::SoftShadows(const SoftShadows& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    penumbraRadius(rhs.penumbraRadius)
{
}

int SoftShadows::compare(const Object& rhs_object) const
{
    int result = ShadowSettings::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value(penumbraRadius, rhs.penumbraRadius);
}

void SoftShadows::read(Input& input)
{
    ShadowSettings::read(input);

    input.read("penumbraRadius", penumbraRadius);
}

void SoftShadows::write(Output& output) const
{
    ShadowSettings::write(output);

    output.write("penumbraRadius", penumbraRadius);
}
