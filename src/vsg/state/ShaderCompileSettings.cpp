/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/read.h>
#include <vsg/state/ShaderCompileSettings.h>

using namespace vsg;

ShaderCompileSettings::ShaderCompileSettings()
{
}

ShaderCompileSettings::ShaderCompileSettings(const ShaderCompileSettings& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    vulkanVersion(rhs.vulkanVersion),
    clientInputVersion(rhs.clientInputVersion),
    language(rhs.language),
    defaultVersion(rhs.defaultVersion),
    target(rhs.target),
    forwardCompatible(rhs.forwardCompatible),
    generateDebugInfo(rhs.generateDebugInfo),
    optimize(rhs.optimize),
    defines(rhs.defines)
{
}

int ShaderCompileSettings::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(vulkanVersion, rhs.vulkanVersion))) return result;
    if ((result = compare_value(clientInputVersion, rhs.clientInputVersion))) return result;
    if ((result = compare_value(language, rhs.language))) return result;
    if ((result = compare_value(defaultVersion, rhs.defaultVersion))) return result;
    if ((result = compare_value(target, rhs.target))) return result;
    if ((result = compare_value(forwardCompatible, rhs.forwardCompatible))) return result;
    if ((result = compare_value(generateDebugInfo, rhs.generateDebugInfo))) return result;
    if ((result = compare_value(optimize, rhs.optimize))) return result;
    return compare_map(defines.values, rhs.defines.values);
}

void ShaderCompileSettings::read(Input& input)
{
    input.read("vulkanVersion", vulkanVersion);
    input.read("clientInputVersion", clientInputVersion);
    input.readValue<int>("language", language);
    input.read("defaultVersion", defaultVersion);
    input.readValue<int>("target", target);
    input.read("forwardCompatible", forwardCompatible);

    if (input.version_greater_equal(1, 0, 4))
    {
        input.read("generateDebugInfo", generateDebugInfo);
    }

    if (input.version_greater_equal(1, 1, 11))
    {
        input.read("optimize", optimize);
    }

    if (input.version_greater_equal(1, 1, 18))
    {
        input.readValues("defines", defines.values);
    }
    else
    {
        std::set<std::string> old_defines;
        input.readValues("defines", old_defines);
        defines = old_defines;
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

    if (output.version_greater_equal(1, 0, 4))
    {
        output.write("generateDebugInfo", generateDebugInfo);
    }

    if (output.version_greater_equal(1, 1, 11))
    {
        output.write("optimize", optimize);
    }

    if (output.version_greater_equal(1, 1, 18))
    {
        output.writeValues("defines", defines.values);
    }
    else
    {
        // Older version of VSG don't support map, so convert to std::set just retaining the key
        output.writeValues("defines", defines.keys());
    }
}
