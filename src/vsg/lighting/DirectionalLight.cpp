/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/lighting/DirectionalLight.h>

using namespace vsg;

DirectionalLight::DirectionalLight()
{
}

DirectionalLight::DirectionalLight(const DirectionalLight& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    direction(rhs.direction),
    angleSubtended(rhs.angleSubtended)
{
}

int DirectionalLight::compare(const Object& rhs_object) const
{
    int result = Light::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(direction, rhs.direction)) != 0) return result;
    return compare_value(angleSubtended, rhs.angleSubtended);
}

void DirectionalLight::read(Input& input)
{
    Light::read(input);

    input.read("direction", direction);

    if (input.version_greater_equal(1, 1, 2))
    {
        input.read("angleSubtended", angleSubtended);
    }
}

void DirectionalLight::write(Output& output) const
{
    Light::write(output);

    output.write("direction", direction);

    if (output.version_greater_equal(1, 1, 2))
    {
        output.write("angleSubtended", angleSubtended);
    }
}
