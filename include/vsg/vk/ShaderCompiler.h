#pragma once

#include <vsg/io/FileSystem.h>
#include <vsg/state/ShaderStage.h>

namespace vsg
{
    class VSG_DECLSPEC ShaderCompiler : public vsg::Inherit<vsg::Object, ShaderCompiler>
    {
    public:
        ShaderCompiler(vsg::Allocator* allocator=nullptr);
        virtual ~ShaderCompiler();

        bool compile(vsg::ShaderStages& shaders, const std::vector<std::string>& defines = {}, const vsg::Paths& paths = {});

        std::string combineSourceAndDefines(const std::string& source, const std::vector<std::string>& defines);
        std::string insertIncludes(const std::string& source, const vsg::Paths& paths);
        std::string readShaderSource(const vsg::Path& filename, const vsg::Paths& paths);
    };
}
