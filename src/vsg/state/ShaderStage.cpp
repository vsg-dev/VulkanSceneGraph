/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/ShaderStage.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/io/Options.h>

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

ref_ptr<ShaderStage> ShaderStage::read(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename)
{
    return ShaderStage::create(stage, entryPointName, ShaderModule::read(filename));
}

void ShaderStage::read(Input& input)
{
    Object::read(input);

    input.readValue<int32_t>("Stage", _stage);

    input.read("EntryPoint", _entryPointName);

    input.readObject("ShaderModule", _shaderModule);

    _specializationConstants.clear();
    uint32_t numValues = input.readValue<uint32_t>("NumSpecializationConstants");
    for (uint32_t i = 0; i < numValues; ++i)
    {
        uint32_t id = input.readValue<uint32_t>("constantID");
        input.readObject("data", _specializationConstants[id]);
    }
}

void ShaderStage::write(Output& output) const
{
    Object::write(output);

    output.writeValue<int32_t>("Stage", _stage);

    output.write("EntryPoint", _entryPointName);

    output.writeObject("ShaderModule", _shaderModule.get());

    output.writeValue<uint32_t>("NumSpecializationConstants", _specializationConstants.size());
    for (auto& [id, data] : _specializationConstants)
    {
        output.writeValue<uint32_t>("constantID", id);
        output.writeObject("data", data);
    }
}

void ShaderStage::apply(Context& context, VkPipelineShaderStageCreateInfo& stageInfo) const
{
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = _stage;
    stageInfo.module = _shaderModule->vk(context.deviceID);
    stageInfo.pName = _entryPointName.c_str();

    if (_specializationConstants.empty())
    {
        stageInfo.pSpecializationInfo = nullptr;
    }
    else
    {
        uint32_t packedDataSize = 0;
        for (auto& id_data : _specializationConstants)
        {
            packedDataSize += static_cast<uint32_t>(id_data.second->dataSize());
        }

        // allocate temporary memoory to pack the specialization map and data into.
        auto mapEntries = context.scratchMemory->allocate<VkSpecializationMapEntry>(_specializationConstants.size());
        auto packedData = context.scratchMemory->allocate<uint8_t>(packedDataSize);
        uint32_t offset = 0;
        uint32_t i = 0;
        for (auto& [id, data] : _specializationConstants)
        {
            mapEntries[i++] = VkSpecializationMapEntry{id, offset, data->dataSize()};
            std::memcpy(packedData + offset, static_cast<uint8_t*>(data->dataPointer()), data->dataSize());
            offset += static_cast<uint32_t>(data->dataSize());
        }

        auto specializationInfo = context.scratchMemory->allocate<VkSpecializationInfo>(1);

        stageInfo.pSpecializationInfo = specializationInfo;

        // assign the values from the ShaderStage into the specializationInfo
        specializationInfo->mapEntryCount = static_cast<uint32_t>(_specializationConstants.size());
        specializationInfo->pMapEntries = mapEntries;
        specializationInfo->dataSize = packedDataSize;
        specializationInfo->pData = packedData;
    }
}

void ShaderStage::compile(Context& context)
{
    if (_shaderModule) _shaderModule->compile(context);
}
