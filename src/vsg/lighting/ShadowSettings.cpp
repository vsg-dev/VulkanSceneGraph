/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/lighting/Light.h>
#include <vsg/nodes/AbsoluteTransform.h>

using namespace vsg;

ShadowSettings::ShadowSettings(uint32_t in_shadowMapCount) :
    shadowMapCount(in_shadowMapCount)
{
}

ShadowSettings::ShadowSettings(const ShadowSettings& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    shadowMapCount(rhs.shadowMapCount)
{
}

int ShadowSettings::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value(shadowMapCount, rhs.shadowMapCount);
}

void ShadowSettings::read(Input& input)
{
    input.read("shadowMapCount", shadowMapCount);
}

void ShadowSettings::write(Output& output) const
{
    output.write("shadowMapCount", shadowMapCount);
}
