/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/glsl.h>
#include <vsg/state/ShaderStage.h>

using namespace vsg;

namespace
{
    // set up the static s_extensionToStage so that it can be used for extension/feature checks
    std::map<Path, VkShaderStageFlagBits> s_extensionToStage{
        {".vert", VK_SHADER_STAGE_VERTEX_BIT},
        {".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
        {".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
        {".geom", VK_SHADER_STAGE_GEOMETRY_BIT},
        {".frag", VK_SHADER_STAGE_FRAGMENT_BIT},
        {".comp", VK_SHADER_STAGE_COMPUTE_BIT},
        {".mesh", VK_SHADER_STAGE_MESH_BIT_EXT},
        {".task", VK_SHADER_STAGE_TASK_BIT_EXT},
        {".rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR},
        {".rint", VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
        {".rahit", VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
        {".rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
        {".rmiss", VK_SHADER_STAGE_MISS_BIT_KHR},
        {".rcall", VK_SHADER_STAGE_CALLABLE_BIT_KHR},
        {".glsl", VK_SHADER_STAGE_ALL},
        {".hlsl", VK_SHADER_STAGE_ALL}};
} // namespace

glsl::glsl()
{
}

bool glsl::extensionSupported(const Path& ext)
{
    return s_extensionToStage.find(ext) != s_extensionToStage.end();
}

ref_ptr<Object> glsl::createShader(const Path& found_filename, std::string& source, VkShaderStageFlagBits stageFlagBits, ref_ptr<const Options> options) const
{
    // handle any #includes in the source
    if (source.find("include") != std::string::npos)
    {
        source = insertIncludes(source, prependPathToOptionsIfRequired(found_filename, options));
    }

    auto sm = ShaderModule::create(source);

    if (stageFlagBits != VK_SHADER_STAGE_ALL)
    {
        return ShaderStage::create(stageFlagBits, "main", sm);
    }

    return sm;
}

ref_ptr<Object> glsl::read(const Path& filename, ref_ptr<const Options> options) const
{
    auto stage_itr = (options && options->extensionHint) ? s_extensionToStage.find(options->extensionHint) : s_extensionToStage.end();
    if (stage_itr == s_extensionToStage.end()) stage_itr = s_extensionToStage.find(lowerCaseFileExtension(filename));
    if (stage_itr == s_extensionToStage.end()) return {};

    Path found_filename = findFile(filename, options);
    if (!found_filename) return {};

    std::ifstream fin(found_filename, std::ios::ate | std::ios::binary);
    fin.seekg(0, fin.end);
    size_t fileSize = fin.tellg();

    std::string source(fileSize, ' ');

    fin.seekg(0);
    fin.read(source.data(), fileSize);
    fin.close();

    return createShader(found_filename, source, stage_itr->second, options);
}

ref_ptr<vsg::Object> glsl::read(std::istream& fin, ref_ptr<const Options> options) const
{
    auto stage_itr = (options && options->extensionHint) ? s_extensionToStage.find(options->extensionHint) : s_extensionToStage.end();
    if (stage_itr == s_extensionToStage.end()) return {};

    fin.seekg(0, fin.end);
    size_t fileSize = fin.tellg();

    std::string source(fileSize, ' ');

    fin.seekg(0);
    fin.read(source.data(), fileSize);

    return createShader({}, source, stage_itr->second, options);
}

ref_ptr<vsg::Object> glsl::read(const uint8_t* ptr, size_t size, ref_ptr<const Options> options) const
{
    auto stage_itr = (options && options->extensionHint) ? s_extensionToStage.find(options->extensionHint) : s_extensionToStage.end();
    if (stage_itr == s_extensionToStage.end()) return {};

    std::string source(reinterpret_cast<const char*>(ptr), size);

    return createShader({}, source, stage_itr->second, options);
}

bool glsl::write(const Object* object, const Path& filename, ref_ptr<const Options> options) const
{
    auto stage_itr = (options && options->extensionHint) ? s_extensionToStage.find(options->extensionHint) : s_extensionToStage.end();
    if (stage_itr == s_extensionToStage.end()) stage_itr = s_extensionToStage.find(lowerCaseFileExtension(filename));
    if (stage_itr == s_extensionToStage.end()) return false;

    const ShaderStage* ss = dynamic_cast<const ShaderStage*>(object);
    const ShaderModule* sm = ss ? ss->module.get() : dynamic_cast<const ShaderModule*>(object);
    if (sm)
    {
        if (!sm->source.empty())
        {
            std::ofstream fout(filename);
            fout.write(sm->source.data(), sm->source.size());
            fout.close();
            return true;
        }
    }
    return false;
}

bool glsl::getFeatures(Features& features) const
{
    for (auto& ext : s_extensionToStage)
    {
        features.extensionFeatureMap[ext.first] = static_cast<ReaderWriter::FeatureMask>(ReaderWriter::READ_FILENAME | ReaderWriter::WRITE_FILENAME);
    }
    return true;
}
