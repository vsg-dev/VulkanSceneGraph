#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/ShaderModule.h>

namespace vsg
{
    // forward declare
    class Context;

    /// ShaderStage encapsulates to VkPipelineShaderStageCreateInfo settings passed when setting up GraphicsPipeline
    class VSG_DECLSPEC ShaderStage : public Inherit<Object, ShaderStage>
    {
    public:
        ShaderStage();
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, ref_ptr<ShaderModule> shaderModule);
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& source, ref_ptr<ShaderCompileSettings> hints = {});
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const ShaderModule::SPIRV& spirv);
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& source, const ShaderModule::SPIRV& spirv);

        using SpecializationConstants = std::map<uint32_t, vsg::ref_ptr<vsg::Data>>;

        /// Vulkan VkPipelineShaderStageCreateInfo settings
        VkPipelineShaderStageCreateFlags flags = 0;
        VkShaderStageFlagBits stage = {};
        ref_ptr<ShaderModule> module;
        std::string entryPointName;
        SpecializationConstants specializationConstants;

        static ref_ptr<ShaderStage> read(VkShaderStageFlagBits stage, const std::string& entryPointName, const Path& filename, ref_ptr<const Options> options = {});
        static ref_ptr<ShaderStage> read(VkShaderStageFlagBits stage, const std::string& entryPointName, std::istream& fin, ref_ptr<const Options> options = {});

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void apply(Context& context, VkPipelineShaderStageCreateInfo& stageInfo) const;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

    protected:
        virtual ~ShaderStage();
    };
    VSG_type_name(vsg::ShaderStage);

    using ShaderStages = std::vector<ref_ptr<ShaderStage>>;

} // namespace vsg
