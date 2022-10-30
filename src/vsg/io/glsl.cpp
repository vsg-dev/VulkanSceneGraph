/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/glsl.h>
#include <vsg/state/ShaderStage.h>

using namespace vsg;

glsl::glsl()
{
}

// set up the static s_extensionToStage so that it can be used for extension/feature checks
std::map<Path, VkShaderStageFlagBits> glsl::s_extensionToStage{
    {".vert", VK_SHADER_STAGE_VERTEX_BIT},
    {".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {".geom", VK_SHADER_STAGE_GEOMETRY_BIT},
    {".frag", VK_SHADER_STAGE_FRAGMENT_BIT},
    {".comp", VK_SHADER_STAGE_COMPUTE_BIT},
    {".mesh", VK_SHADER_STAGE_MESH_BIT_NV},
    {".task", VK_SHADER_STAGE_TASK_BIT_NV},
    {".rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR},
    {".rint", VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
    {".rahit", VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
    {".rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
    {".rmiss", VK_SHADER_STAGE_MISS_BIT_KHR},
    {".rcall", VK_SHADER_STAGE_CALLABLE_BIT_KHR},
    {".glsl", VK_SHADER_STAGE_ALL},
    {".hlsl", VK_SHADER_STAGE_ALL}};

bool glsl::extensionSupported(const Path& ext)
{
    return s_extensionToStage.find(ext) != s_extensionToStage.end();
}

ref_ptr<Object> glsl::read(const Path& filename, ref_ptr<const Options> options) const
{
    auto ext = lowerCaseFileExtension(filename);
    auto stage_itr = s_extensionToStage.find(ext);
    if (stage_itr != s_extensionToStage.end())
    {
        Path found_filename = findFile(filename, options);
        if (!found_filename) return {};

        std::string source;

        std::ifstream fin(found_filename, std::ios::ate | std::ios::binary);
        size_t fileSize = fin.tellg();

        source.resize(fileSize);

        fin.seekg(0);
        fin.read(reinterpret_cast<char*>(source.data()), fileSize);
        fin.close();

        // handle any #includes in the source
        if (source.find("include") != std::string::npos)
        {
            source = insertIncludes(source, prependPathToOptionsIfRequired(found_filename, options));
        }

        auto sm = ShaderModule::create(source);

        if (stage_itr->second == VK_SHADER_STAGE_ALL)
        {
            return sm;
        }
        else
        {
            return ShaderStage::create(stage_itr->second, "main", sm);
        }

        return sm;
    }
    return {};
}

bool glsl::write(const Object* object, const Path& filename, ref_ptr<const Options> /*options*/) const
{
    auto ext = lowerCaseFileExtension(filename);
    auto stage_itr = s_extensionToStage.find(ext);
    if (stage_itr != s_extensionToStage.end())
    {
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
