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

namespace vsg
{
    // forward declare
    class Context;

    /// Settings passed to glsLang when compiling GLSL/HLSL shader source to SPIR-V
    /// Provides the values to pass to glsLang::TShader::setEnvInput, setEnvClient and setEnvTarget.
    class VSG_DECLSPEC ShaderCompileSettings : public Inherit<Object, ShaderCompileSettings>
    {
    public:
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

        std::set<std::string> defines;

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::ShaderCompileSettings);

    /// ShaderModule encapsulates the VkShaderModule and the VkShaderModuleCreateInfo settings used to set it up.
    /// ShaderModule are assigned to ShaderStage, which are assigned to GraphicsPipeline/ComputePipeline etc.
    class VSG_DECLSPEC ShaderModule : public Inherit<Object, ShaderModule>
    {
    public:
        using SPIRV = std::vector<uint32_t>;

        ShaderModule();
        explicit ShaderModule(const std::string& in_source, ref_ptr<ShaderCompileSettings> in_hints = {});
        explicit ShaderModule(const SPIRV& in_code);
        ShaderModule(const std::string& source, const SPIRV& in_code);

        std::string source;
        ref_ptr<ShaderCompileSettings> hints;

        /// VkShaderModuleCreateInfo settings
        SPIRV code;

        /// Vulkan VkShaderModule handle
        VkShaderModule vk(uint32_t deviceID) const { return _implementation[deviceID]->_shaderModule; }

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

    protected:
        virtual ~ShaderModule();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Device* device, ShaderModule* shader);

            virtual ~Implementation();

            VkShaderModule _shaderModule;
            ref_ptr<Device> _device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::ShaderModule);

    /// replace all instances of #include "filename.extension" with the contents of the related files.
    extern VSG_DECLSPEC std::string insertIncludes(const std::string& source, ref_ptr<const Options> options);

} // namespace vsg
