/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/nodes/AbsoluteTransform.h>

using namespace vsg;

Light::Light()
{
}

Light::Light(const Light& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    name(rhs.name),
    color(rhs.color),
    intensity(rhs.intensity),
    shadowSettings(rhs.shadowSettings)
{
}

int Light::compare(const Object& rhs_object) const
{
    int result = Node::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(name, rhs.name)) != 0) return result;
    if ((result = compare_value(color, rhs.color)) != 0) return result;
    if ((result = compare_value(intensity, rhs.intensity)) != 0) return result;
    return compare_pointer(shadowSettings, rhs.shadowSettings);
}

void Light::read(Input& input)
{
    input.read("name", name);
    input.read("color", color);
    input.read("intensity", intensity);

    if (input.version_greater_equal(1, 1, 3))
    {
        input.read("shadowSettings", shadowSettings);
    }
}

void Light::write(Output& output) const
{
    output.write("name", name);
    output.write("color", color);
    output.write("intensity", intensity);

    if (output.version_greater_equal(1, 1, 3))
    {
        output.write("shadowSettings", shadowSettings);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Headlight
//
ref_ptr<vsg::Node> vsg::createHeadlight()
{
    auto ambientLight = vsg::AmbientLight::create();
    ambientLight->name = "ambient";
    ambientLight->color.set(1.0f, 1.0f, 1.0f);
    ambientLight->intensity = 0.0044f;

    auto directionalLight = vsg::DirectionalLight::create();
    directionalLight->name = "headlight";
    directionalLight->color.set(1.0f, 1.0f, 1.0f);
    directionalLight->intensity = 0.9956f;
    directionalLight->direction.set(0.0, 0.0, -1.0);

    auto absoluteTransform = vsg::AbsoluteTransform::create();
    absoluteTransform->addChild(ambientLight);
    absoluteTransform->addChild(directionalLight);
    return absoluteTransform;
}
