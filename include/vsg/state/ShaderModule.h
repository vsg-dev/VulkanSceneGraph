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
#include <vsg/state/ShaderCompileSettings.h>

namespace vsg
{
    // forward declare
    class Context;

    /// ShaderModule encapsulates the VkShaderModule and the VkShaderModuleCreateInfo settings used to set it up.
    /// ShaderModule are assigned to ShaderStage, which are assigned to GraphicsPipeline/ComputePipeline etc.
    class VSG_DECLSPEC ShaderModule : public Inherit<Object, ShaderModule>
    {
    public:
        using SPIRV = std::vector<uint32_t>;

        ShaderModule();
        ShaderModule(const ShaderModule& rhs, const CopyOp& copyop = {});
        explicit ShaderModule(const std::string& in_source, ref_ptr<ShaderCompileSettings> in_hints = {});
        explicit ShaderModule(const SPIRV& in_code);
        ShaderModule(const std::string& source, const SPIRV& in_code);

        std::string source;
        ref_ptr<ShaderCompileSettings> hints;

        /// VkShaderModuleCreateInfo settings
        SPIRV code;

        /// Vulkan VkShaderModule handle
        VkShaderModule vk(uint32_t deviceID) const { return _implementation[deviceID]->_shaderModule; }

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return ShaderModule::create(*this, copyop); }
        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        ~ShaderModule() override;

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Device* device, ShaderModule* shader);

            ~Implementation() override;

            VkShaderModule _shaderModule;
            ref_ptr<Device> _device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::ShaderModule);

    /// replace all instances of #include "filename.extension" with the contents of the related files.
    extern VSG_DECLSPEC std::string insertIncludes(const std::string& source, ref_ptr<const Options> options);

} // namespace vsg
