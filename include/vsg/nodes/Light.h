#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/common.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    /// Light node a base class for different light types - AmbientLight, DirectionalLight, PointLight and SpotLight.
    /// Used to by the RecordTraversal to represent a light source that is placed in the LightData uniform used by the shaders when implementing lighting.
    /// Provides name, color and intensity settings common to all Light types.
    class VSG_DECLSPEC Light : public Inherit<Node, Light>
    {
    public:
        std::string name;
        vec3 color = vec3(1.0f, 1.0f, 1.0f);
        float intensity = 1.0f;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~Light() {}
    };
    VSG_type_name(vsg::Light);

    /// AmbientLight represents an ambient light source
    class VSG_DECLSPEC AmbientLight : public Inherit<Light, AmbientLight>
    {
    public:
        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~AmbientLight() {}
    };
    VSG_type_name(vsg::AmbientLight);

    /// DirectionalLight represents a directional light source - used for light sources that are treated as if at infinity, like sun/moon.
    class VSG_DECLSPEC DirectionalLight : public Inherit<Light, DirectionalLight>
    {
    public:
        dvec3 direction = dvec3(0.0, 0.0, -1.0);

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~DirectionalLight() {}
    };
    VSG_type_name(vsg::DirectionalLight);

    /// PointLight represents a local point light source where all light radiants event from the light position.
    class VSG_DECLSPEC PointLight : public Inherit<Light, PointLight>
    {
    public:
        dvec3 position = dvec3(0.0, 0.0, 0.0);

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~PointLight() {}
    };
    VSG_type_name(vsg::PointLight);

    /// SpotLight represents a local point light source which intensity varies as a spot light.
    class VSG_DECLSPEC SpotLight : public Inherit<Light, SpotLight>
    {
    public:
        dvec3 position = dvec3(0.0, 0.0, 0.0);
        dvec3 direction = dvec3(0.0, 0.0, -1.0);
        double innerAngle = radians(30.0);
        double outerAngle = radians(45.0);

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~SpotLight() {}
    };
    VSG_type_name(vsg::SpotLight);

    /// convenience method for creating a subgraph that creates a headlight illumination using a white AmibientLight and DirectionalLight with intensity of 0.1 and 0.9 respectively.
    extern VSG_DECLSPEC ref_ptr<vsg::Node> createHeadlight();

} // namespace vsg
