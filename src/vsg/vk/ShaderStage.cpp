/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/ShaderStage.h>

using namespace vsg;

ShaderStage::ShaderStage()
{
}

ShaderStage::ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, ref_ptr<ShaderModule> shaderModule) :
    _stage(stage),
    _entryPointName(entryPointName),
    _shaderModule(shaderModule)
{
}

ShaderStage::ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const ShaderModule::Source& source) :
    _stage(stage),
    _entryPointName(entryPointName),
    _shaderModule(ShaderModule::create(source))
{
}

ShaderStage::ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const ShaderModule::SPIRV& spirv) :
    _stage(stage),
    _entryPointName(entryPointName),
    _shaderModule(ShaderModule::create(spirv))
{
}

ShaderStage::ShaderStage(VkShaderStageFlagBits stage, const std::string& entryPointName, const ShaderModule::Source& source, const ShaderModule::SPIRV& spirv) :
    _stage(stage),
    _entryPointName(entryPointName),
    _shaderModule(ShaderModule::create(source, spirv))
{
}

ShaderStage::Result ShaderStage::read(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename)
{
    ref_ptr<ShaderModule> shaderModule = ShaderModule::read(filename);
    if (shaderModule)
    {
        return Result(new ShaderStage(stage, entryPointName, shaderModule));
    }
    else
    {
        return ShaderStage::Result("Error: vsg::ShaderModule::read(..) failed to read shader file.", VK_INCOMPLETE);
    }
}

void ShaderStage::read(Input& input)
{
    Object::read(input);

    _stage = static_cast<VkShaderStageFlagBits>(input.readValue<int32_t>("Stage"));

    input.read("EntryPoint", _entryPointName);

    _shaderModule = input.readObject<ShaderModule>("ShaderModule");

    _specializationMapEntries.resize(input.readValue<uint32_t>("NumSpecializationMapEntries"));
    for (auto& specializationMapEntry : _specializationMapEntries)
    {
        input.read("constantID", specializationMapEntry.constantID);
        input.read("offset", specializationMapEntry.offset);
        specializationMapEntry.size = input.readValue<uint32_t>("size");
    }

    _specializationData = input.readObject<Data>("SpecializationData");
}

void ShaderStage::write(Output& output) const
{
    Object::write(output);

    output.writeValue<int32_t>("Stage", _stage);

    output.write("EntryPoint", _entryPointName);

    output.writeObject("ShaderModule", _shaderModule.get());

    output.writeValue<uint32_t>("NumSpecializationMapEntries", _specializationMapEntries.size());
    for (auto& specializationMapEntry : _specializationMapEntries)
    {
        output.write("constantID", specializationMapEntry.constantID);
        output.write("offset", specializationMapEntry.offset);
        output.writeValue<uint32_t>("size", specializationMapEntry.size);
    }

    output.writeObject("SpecializationData", _specializationData.get());
}

void ShaderStage::apply(VkPipelineShaderStageCreateInfo& stageInfo) const
{
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = _stage;
    stageInfo.module = *_shaderModule;
    stageInfo.pName = _entryPointName.c_str();
}

void ShaderStage::compile(Context& context)
{
    if (_shaderModule) _shaderModule->compile(context);
}
