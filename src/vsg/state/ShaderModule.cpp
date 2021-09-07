/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/state/ShaderModule.h>
#include <vsg/traversals/CompileTraversal.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ShaderCompileSettings
//
void ShaderCompileSettings::read(Input& input)
{
    input.read("vulkanVersion", vulkanVersion);
    input.read("clientInputVersion", clientInputVersion);
    input.readValue<int>("language", language);
    input.read("defaultVersion", defaultVersion);
    input.readValue<int>("target", target);
    input.read("forwardCompatible", forwardCompatible);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("defines", defines);
    }
}

void ShaderCompileSettings::write(Output& output) const
{
    output.write("vulkanVersion", vulkanVersion);
    output.write("clientInputVersion", clientInputVersion);
    output.writeValue<int>("language", language);
    output.write("defaultVersion", defaultVersion);
    output.writeValue<int>("target", target);
    output.write("forwardCompatible", forwardCompatible);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("defines", defines);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Shader
//
ShaderModule::ShaderModule()
{
}

ShaderModule::ShaderModule(const std::string& in_source, ref_ptr<ShaderCompileSettings> in_hints) :
    source(in_source),
    hints(in_hints)
{
}

ShaderModule::ShaderModule(const SPIRV& in_code) :
    code(in_code)
{
}

ShaderModule::ShaderModule(const std::string& in_source, const SPIRV& in_code) :
    source(in_source),
    code(in_code)
{
}

ShaderModule::~ShaderModule()
{
}

void ShaderModule::read(Input& input)
{
    Object::read(input);

    // TODO review IO
    input.read("Source", source);

    if (input.version_greater_equal(0, 1, 3))
    {
        input.readObject("hints", hints);
    }

    code.resize(input.readValue<uint32_t>("SPIRVSize"));

    input.matchPropertyName("SPIRV");
    input.read(code.size(), code.data());
}

void ShaderModule::write(Output& output) const
{
    Object::write(output);

    output.write("Source", source);

    if (output.version_greater_equal(0, 1, 3))
    {
        output.writeObject("hints", hints);
    }

    output.writeValue<uint32_t>("SPIRVSize", code.size());

    output.writePropertyName("SPIRV");
    output.write(code.size(), code.data());
    output.writeEndOfLine();
}

void ShaderModule::compile(Context& context)
{
    if (!_implementation[context.deviceID]) _implementation[context.deviceID] = Implementation::create(context.device, this);
}

ShaderModule::Implementation::Implementation(Device* device, ShaderModule* shaderModule) :
    _device(device)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderModule->code.size() * sizeof(ShaderModule::SPIRV::value_type);
    createInfo.pCode = shaderModule->code.data();
    createInfo.pNext = nullptr;

    if (VkResult result = vkCreateShaderModule(*device, &createInfo, _device->getAllocationCallbacks(), &_shaderModule); result != VK_SUCCESS)
    {
        throw Exception{"Error: vsg::ShaderModule::create(...) failed to create shader module.", result};
    }
}

ShaderModule::Implementation::~Implementation()
{
    vkDestroyShaderModule(*_device, _shaderModule, _device->getAllocationCallbacks());
}
