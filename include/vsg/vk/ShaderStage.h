#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <fstream>
#include <vsg/vk/ShaderModule.h>

namespace vsg
{
    // forward declare
    class Context;

    class VSG_DECLSPEC ShaderStage : public Inherit<Object, ShaderStage>
    {
    public:
        ShaderStage();
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, ref_ptr<ShaderModule> shaderModule);
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const ShaderModule::Source& source);
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const ShaderModule::SPIRV& spirv);
        ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const ShaderModule::Source& source, const ShaderModule::SPIRV& spirv);

        void setShaderModule(ShaderModule* shaderModule) { _shaderModule = shaderModule; }
        ShaderModule* getShaderModule() { return _shaderModule; }
        const ShaderModule* getShaderModule() const { return _shaderModule; }

        void setShaderStageFlagBits(VkShaderStageFlagBits flags) { _stage = flags; }
        VkShaderStageFlagBits getShaderStageFlagBits() const { return _stage; }

        void setEntryPointName(std::string& entryPointName) { _entryPointName = entryPointName; }
        const std::string& getEntryPointName() const { return _entryPointName; }

        using SpecializationConstants = std::map<uint32_t, vsg::ref_ptr<vsg::Data>>;

        void setSpecializationConstants(const SpecializationConstants& sc) { _specializationConstants = sc; }
        SpecializationConstants& getSpecializationConstants() { return _specializationConstants; }
        const SpecializationConstants& getSpecializationConstants() const { return _specializationConstants; }

        using Result = vsg::Result<ShaderStage, VkResult, VK_SUCCESS>;
        static Result read(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename);

        void read(Input& input) override;
        void write(Output& output) const override;

        void apply(Context& context, VkPipelineShaderStageCreateInfo& stageInfo) const;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

    protected:
        VkShaderStageFlagBits _stage;
        std::string _entryPointName;
        ref_ptr<ShaderModule> _shaderModule;
        SpecializationConstants _specializationConstants;
    };
    VSG_type_name(vsg::ShaderStage);

    using ShaderStages = std::vector<ref_ptr<ShaderStage>>;

} // namespace vsg
