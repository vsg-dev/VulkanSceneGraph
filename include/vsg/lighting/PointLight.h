#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/lighting/Light.h>
#include <vsg/maths/common.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    /// PointLight represents a local point light source where all light radiants event from the light position.
    class VSG_DECLSPEC PointLight : public Inherit<Light, PointLight>
    {
    public:
        PointLight();
        PointLight(const PointLight& rhs, const CopyOp& copyop = {});

        dvec3 position = dvec3(0.0, 0.0, 0.0);
        double radius = 0.0f;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return PointLight::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~PointLight() {}
    };
    VSG_type_name(vsg::PointLight);

} // namespace vsg
