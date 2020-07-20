/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/state/ShaderModule.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/io/Options.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Shader
//
ShaderModule::ShaderModule()
{
}

ShaderModule::ShaderModule(const Source& source) :
    _source(source)
{
}

ShaderModule::ShaderModule(const SPIRV& spirv) :
    _spirv(spirv)
{
}

ShaderModule::ShaderModule(const Source& source, const SPIRV& spirv) :
    _source(source),
    _spirv(spirv)
{
}

ShaderModule::~ShaderModule()
{
}

ref_ptr<ShaderModule> ShaderModule::read(const std::string& filename)
{
    SPIRV buffer;
    if (readFile(buffer, filename))
    {
        return ShaderModule::create(buffer);
    }
    else
    {
        throw Exception{"Error: vsg::ShaderModule::read(..) failed to read shader file.", VK_INCOMPLETE};
    }
}

void ShaderModule::read(Input& input)
{
    Object::read(input);

    input.read("Source", _source);

    _spirv.resize(input.readValue<uint32_t>("SPIRVSize"));

    input.matchPropertyName("SPIRV");
    input.read(_spirv.size(), _spirv.data());
}

void ShaderModule::write(Output& output) const
{
    Object::write(output);

    output.write("Source", _source);

    output.writeValue<uint32_t>("SPIRVSize", _spirv.size());

    output.writePropertyName("SPIRV");
    output.write(_spirv.size(), _spirv.data());
    output.writeEndOfLine();
}

void ShaderModule::compile(Context& context)
{
    if (!_implementation[context.deviceID]) _implementation[context.deviceID] = Implementation::create(context.device, this);
}

ShaderModule::Implementation::Implementation(Device* device, ShaderModule* shaderModule, AllocationCallbacks* allocator) :
    _device(device),
    _allocator(allocator)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderModule->spirv().size() * sizeof(ShaderModule::SPIRV::value_type);
    createInfo.pCode = shaderModule->spirv().data();
    createInfo.pNext = nullptr;

    if (VkResult result = vkCreateShaderModule(*device, &createInfo, allocator, &_shaderModule); result != VK_SUCCESS)
    {
        throw Exception{"Error: vsg::ShaderModule::create(...) failed to create shader module.", result};
    }
}

ShaderModule::Implementation::~Implementation()
{
    vkDestroyShaderModule(*_device, _shaderModule, _allocator);
}
