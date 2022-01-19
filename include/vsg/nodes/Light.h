#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/transform.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    class VSG_DECLSPEC Light : public Inherit<Node, Light>
    {
    public:
        std::string name;
        vec3 color = vec3(1.0, 1.0, 1.0);
        float intensity = 1.0;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~Light() {}
    };
    VSG_type_name(vsg::Light);

    class VSG_DECLSPEC AmbientLight : public Inherit<Light, AmbientLight>
    {
    public:

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~AmbientLight() {}
    };
    VSG_type_name(vsg::AmbientLight);

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

} // namespace vsg
