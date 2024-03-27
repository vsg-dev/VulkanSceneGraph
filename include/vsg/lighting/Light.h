#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/lighting/ShadowSettings.h>
#include <vsg/maths/common.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    /// Light is a base node class for different light types - AmbientLight, DirectionalLight, PointLight and SpotLight.
    /// Used by the RecordTraversal to represent a light source that is placed in the LightData uniform used by the shaders when implementing lighting.
    /// Provides name, color and intensity settings common to all Light types.
    class VSG_DECLSPEC Light : public Inherit<Node, Light>
    {
    public:
        Light();
        Light(const Light& rhs, const CopyOp& copyop = {});

        std::string name;
        vec3 color = vec3(1.0f, 1.0f, 1.0f);
        float intensity = 1.0f;
        ref_ptr<ShadowSettings> shadowSettings;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return Light::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~Light() {}
    };
    VSG_type_name(vsg::Light);

    /// convenience method for creating a subgraph with a headlight illumination using a white AmbientLight and DirectionalLight with intensity of 0.05 and 0.95 respectively.
    extern VSG_DECLSPEC ref_ptr<vsg::Node> createHeadlight();

} // namespace vsg
