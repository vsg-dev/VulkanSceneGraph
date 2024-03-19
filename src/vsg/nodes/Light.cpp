/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/nodes/AbsoluteTransform.h>
#include <vsg/nodes/Light.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Light
//
Light::Light()
{
}

Light::Light(const Light& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    name(rhs.name),
    color(rhs.color),
    intensity(rhs.intensity),
    shadowMaps(rhs.shadowMaps)
{
}

int Light::compare(const Object& rhs_object) const
{
    int result = Node::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(name, rhs.name)) != 0) return result;
    if ((result = compare_value(color, rhs.color)) != 0) return result;
    if ((result = compare_value(intensity, rhs.intensity)) != 0) return result;
    return compare_value(shadowMaps, rhs.shadowMaps);
}

void Light::read(Input& input)
{
    input.read("name", name);
    input.read("color", color);
    input.read("intensity", intensity);
}

void Light::write(Output& output) const
{
    output.write("name", name);
    output.write("color", color);
    output.write("intensity", intensity);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AmbientLight
//
AmbientLight::AmbientLight()
{
}

AmbientLight::AmbientLight(const AmbientLight& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop)
{
}

void AmbientLight::read(Input& input)
{
    Light::read(input);
}

void AmbientLight::write(Output& output) const
{
    Light::write(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DirectionalLight
//
DirectionalLight::DirectionalLight()
{
}

DirectionalLight::DirectionalLight(const DirectionalLight& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    direction(rhs.direction),
    angleSubtended(rhs.angleSubtended),
    fixedPcfRadius(rhs.fixedPcfRadius)
{
}

int DirectionalLight::compare(const Object& rhs_object) const
{
    int result = Light::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(direction, rhs.direction)) != 0) return result;
    if ((result = compare_value(angleSubtended, rhs.angleSubtended)) != 0) return result;
    return compare_value(fixedPcfRadius, rhs.fixedPcfRadius);
}

void DirectionalLight::read(Input& input)
{
    Light::read(input);

    input.read("direction", direction);

    if (input.version_greater_equal(1, 1, 2))
    {
        input.read("angleSubtended", angleSubtended);
        input.read("fixedPcfRadius", fixedPcfRadius);
    }
}

void DirectionalLight::write(Output& output) const
{
    Light::write(output);

    output.write("direction", direction);

    if (output.version_greater_equal(1, 1, 2))
    {
        output.write("angleSubtended", angleSubtended);
        output.write("fixedPcfRadius", fixedPcfRadius);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PointLight
//
PointLight::PointLight()
{
}

PointLight::PointLight(const PointLight& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    position(rhs.position)
{
}

int PointLight::compare(const Object& rhs_object) const
{
    int result = Light::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value(position, rhs.position);
}

void PointLight::read(Input& input)
{
    Light::read(input);

    input.read("position", position);
}

void PointLight::write(Output& output) const
{
    Light::write(output);

    output.write("position", position);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SpotLight
//
SpotLight::SpotLight()
{
}

SpotLight::SpotLight(const SpotLight& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    position(rhs.position),
    direction(rhs.direction),
    innerAngle(rhs.innerAngle),
    outerAngle(rhs.outerAngle)
{
}

int SpotLight::compare(const Object& rhs_object) const
{
    int result = Light::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(position, rhs.position)) != 0) return result;
    if ((result = compare_value(direction, rhs.direction)) != 0) return result;
    if ((result = compare_value(innerAngle, rhs.innerAngle)) != 0) return result;
    return compare_value(outerAngle, rhs.outerAngle);
}

void SpotLight::read(Input& input)
{
    Light::read(input);

    input.read("position", position);
    input.read("direction", direction);
    input.read("innerAngle", innerAngle);
    input.read("outerAngle", outerAngle);
}

void SpotLight::write(Output& output) const
{
    Light::write(output);

    output.write("position", position);
    output.write("direction", direction);
    output.write("innerAngle", innerAngle);
    output.write("outerAngle", outerAngle);
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
    ambientLight->intensity = 0.05f;

    auto directionalLight = vsg::DirectionalLight::create();
    directionalLight->name = "headlight";
    directionalLight->color.set(1.0f, 1.0f, 1.0f);
    directionalLight->intensity = 0.95f;
    directionalLight->direction.set(0.0, 0.0, -1.0);

    auto absoluteTransform = vsg::AbsoluteTransform::create();
    absoluteTransform->addChild(ambientLight);
    absoluteTransform->addChild(directionalLight);
    return absoluteTransform;
}
