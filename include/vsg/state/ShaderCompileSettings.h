#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <fstream>

#include <vsg/vk/Device.h>
#include <vsg/vk/vk_buffer.h>
#include <vsg/state/DefineMap.h>

namespace vsg
{

    /// Settings passed to glsLang when compiling GLSL/HLSL shader source to SPIR-V
    /// Provides the values to pass to glsLang::TShader::setEnvInput, setEnvClient and setEnvTarget.
    class VSG_DECLSPEC ShaderCompileSettings : public Inherit<Object, ShaderCompileSettings>
    {
    public:
        ShaderCompileSettings();
        ShaderCompileSettings(const ShaderCompileSettings& rhs, const CopyOp& copyop = {});

        enum Language
        {
            GLSL,
            HLSL
        };

        enum SpirvTarget
        {
            SPIRV_1_0 = (1 << 16),
            SPIRV_1_1 = (1 << 16) | (1 << 8),
            SPIRV_1_2 = (1 << 16) | (2 << 8),
            SPIRV_1_3 = (1 << 16) | (3 << 8),
            SPIRV_1_4 = (1 << 16) | (4 << 8),
            SPIRV_1_5 = (1 << 16) | (5 << 8)
        };

        uint32_t vulkanVersion = VK_API_VERSION_1_0;
        int clientInputVersion = 100;
        Language language = GLSL;
        int defaultVersion = 450;
        SpirvTarget target = SPIRV_1_0;
        bool forwardCompatible = false;
        bool generateDebugInfo = false; // maps to SpvOptions::generateDebugInfo
        bool optimize = false;

        DefineMap defines;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return ShaderCompileSettings::create(*this, copyop); }

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::ShaderCompileSettings);

} // namespace vsg
