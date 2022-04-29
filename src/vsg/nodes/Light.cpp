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
void DirectionalLight::read(Input& input)
{
    Light::read(input);

    input.read("direction", direction);
}

void DirectionalLight::write(Output& output) const
{
    Light::write(output);

    output.write("direction", direction);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PointLight
//
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
