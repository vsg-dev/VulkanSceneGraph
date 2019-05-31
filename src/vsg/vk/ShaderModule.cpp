/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/ShaderModule.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Shader
//
ShaderModule::ShaderModule()
{
}

ShaderModule::ShaderModule(VkShaderStageFlagBits stage, const std::string& entryPointName, const SPIRV& spirv) :
    _stage(stage),
    _entryPointName(entryPointName),
    _spirv(spirv)
{
}

ShaderModule::ShaderModule(VkShaderStageFlagBits stage, const std::string& entryPointName, const Source& source) :
    _stage(stage),
    _entryPointName(entryPointName),
    _source(source)
{
}

ShaderModule::ShaderModule(VkShaderStageFlagBits stage, const std::string& entryPointName, const Source& source, const SPIRV& spirv) :
    _stage(stage),
    _entryPointName(entryPointName),
    _source(source),
    _spirv(spirv)
{
}

ShaderModule::~ShaderModule()
{
}

ShaderModule::Result ShaderModule::read(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename)
{
    SPIRV buffer;
    if (readFile(buffer, filename))
    {
        return Result(new ShaderModule(stage, entryPointName, buffer));
    }
    else
    {
        return ShaderModule::Result("Error: vsg::ShaderModule::read(..) failed to read shader file.", VK_INCOMPLETE);
    }
}

void ShaderModule::read(Input& input)
{
    Object::read(input);

    _stage = static_cast<VkShaderStageFlagBits>(input.readValue<int32_t>("Stage"));

    input.read("EntryPoint", _entryPointName);
    input.read("Source", _source);

    _spirv.resize(input.readValue<uint32_t>("SPIRVSize"));
    input.matchPropertyName("SPIRV");
    input.read(_spirv.size(), _spirv.data());
}

void ShaderModule::write(Output& output) const
{
    Object::write(output);

    output.writeValue<int32_t>("Stage", _stage);

    output.write("EntryPoint", _entryPointName);
    output.write("Source", _source);

    output.writeValue<uint32_t>("SPIRVSize", _spirv.size());
    output.writePropertyName("SPIRV");
    output.write(_spirv.size(), _spirv.data());
}

void ShaderModule::compile(Context& context)
{
    if (!_implementation) _implementation = Implementation::create(context.device, this);
}

ShaderModule::Implementation::Implementation(VkShaderModule shaderModule, Device* device, AllocationCallbacks* allocator) :
    _shaderModule(shaderModule),
    _device(device),
    _allocator(allocator)
{
}

ShaderModule::Implementation::~Implementation()
{
    vkDestroyShaderModule(*_device, _shaderModule, _allocator);
}

ShaderModule::Implementation::Result ShaderModule::Implementation::create(Device* device, ShaderModule* shaderModule, AllocationCallbacks* allocator)
{
    if (!device || !shaderModule)
    {
        return Result("Error: vsg::ShaderModule::create(...) failed, requires valid logical device and Shader.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    if (shaderModule->spirv().empty())
    {
        return Result("Error: vsg::ShaderModule::create(...) failed. requires Shader with valid spirv contents.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderModule->spirv().size() * sizeof(ShaderModule::SPIRV::value_type);
    createInfo.pCode = shaderModule->spirv().data();
    createInfo.pNext = nullptr;

    VkShaderModule sm;
    VkResult result = vkCreateShaderModule(*device, &createInfo, allocator, &sm);
    if (result == VK_SUCCESS)
    {
        return Result(new ShaderModule::Implementation(sm, device, allocator));
    }
    else
    {
        return Result("Error: vsg::ShaderModule::create(...) failed to create shader module.", result);
    }
}
