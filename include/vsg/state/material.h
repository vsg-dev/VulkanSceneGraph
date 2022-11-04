#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/core/Value.h>

namespace vsg
{

    /// simple material struct for passing material settings as uniform value to fragment shader
    struct material
    {
        vec4 ambientColor;
        vec4 diffuseColor;
        vec4 specularColor;
        float shininess{100.0f};

        void read(vsg::Input& input)
        {
            input.read("ambientColor", ambientColor);
            input.read("diffuseColor", diffuseColor);
            input.read("specularColor", specularColor);
            input.read("shininess", shininess);
        }

        void write(vsg::Output& output) const
        {
            output.write("ambientColor", ambientColor);
            output.write("diffuseColor", diffuseColor);
            output.write("specularColor", specularColor);
            output.write("shininess", shininess);
        }
    };

    template<>
    constexpr bool has_read_write<material>() { return true; }

    VSG_value(materialValue, material);
    VSG_array(materialArray, material);

    /// PhongMaterial struct for passing material settings, suitable for phong lighting model, as uniform value to fragment shader
    /// Used in conjunction with vsg::createPhongShaderSet().
    struct PhongMaterial
    {
        vec4 ambient{1.0f, 1.0f, 1.0f, 1.0f};
        vec4 diffuse{0.9f, 0.9f, 0.9f, 1.0f};
        vec4 specular{0.2f, 0.2f, 0.2f, 1.0f};
        vec4 emissive{0.0f, 0.0f, 0.0f, 0.0f};
        float shininess{100.0f};
        float alphaMask{1.0f};
        float alphaMaskCutoff{0.5f};

        void read(vsg::Input& input)
        {
            input.read("ambient", ambient);
            input.read("diffuse", diffuse);
            input.read("specular", specular);
            input.read("emissive", emissive);
            input.read("shininess", shininess);
            input.read("alphaMask", alphaMask);
            input.read("alphaMaskCutoff", alphaMaskCutoff);
        }

        void write(vsg::Output& output) const
        {
            output.write("ambient", ambient);
            output.write("diffuse", diffuse);
            output.write("specular", specular);
            output.write("emissive", emissive);
            output.write("shininess", shininess);
            output.write("alphaMask", alphaMask);
            output.write("alphaMaskCutoff", alphaMaskCutoff);
        }
    };

    template<>
    constexpr bool has_read_write<PhongMaterial>() { return true; }

    VSG_value(PhongMaterialValue, PhongMaterial);
    VSG_array(PhongMaterialArray, PhongMaterial);

    /// PbrMaterial struct for passing material settings, suitable for phong lighting model, as uniform value to fragment shader
    /// Used in conjunction with vsg::createPhysicsBasedRenderingShaderSet().
    struct PbrMaterial
    {
        vec4 baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
        vec4 emissiveFactor{0.0f, 0.0f, 0.0f, 1.0f};
        vec4 diffuseFactor{0.9f, 0.9f, 0.9f, 1.0f};
        vec4 specularFactor{0.2f, 0.2f, 0.2f, 1.0f};
        float metallicFactor{1.0f};
        float roughnessFactor{1.0f};
        float alphaMask{1.0f};
        float alphaMaskCutoff{0.5f};

        void read(vsg::Input& input)
        {
            input.read("baseColorFactor", baseColorFactor);
            input.read("emissiveFactor", emissiveFactor);
            input.read("diffuseFactor", diffuseFactor);
            input.read("specularFactor", specularFactor);
            input.read("metallicFactor", metallicFactor);
            input.read("roughnessFactor", roughnessFactor);
            input.read("alphaMask", alphaMask);
            input.read("alphaMaskCutoff", alphaMaskCutoff);
        }

        void write(vsg::Output& output) const
        {
            output.write("baseColorFactor", baseColorFactor);
            output.write("emissiveFactor", emissiveFactor);
            output.write("diffuseFactor", diffuseFactor);
            output.write("specularFactor", specularFactor);
            output.write("metallicFactor", metallicFactor);
            output.write("roughnessFactor", roughnessFactor);
            output.write("alphaMask", alphaMask);
            output.write("alphaMaskCutoff", alphaMaskCutoff);
        }
    };

    template<>
    constexpr bool has_read_write<PbrMaterial>() { return true; }

    VSG_value(PbrMaterialValue, PbrMaterial);
    VSG_array(PbrMaterialArray, PbrMaterial);

} // namespace vsg
