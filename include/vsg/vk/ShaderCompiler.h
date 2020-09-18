#pragma once

#include <vsg/core/Visitor.h>
#include <vsg/io/FileSystem.h>
#include <vsg/state/ShaderStage.h>

namespace vsg
{
    class VSG_DECLSPEC ShaderCompiler : public Inherit<Visitor, ShaderCompiler>
    {
    public:
        ShaderCompiler();
        virtual ~ShaderCompiler();

        bool compile(ShaderStages& shaders, const std::vector<std::string>& defines = {}, const Paths& paths = {});
        bool compile(ref_ptr<ShaderStage> shaderStage, const std::vector<std::string>& defines = {}, const Paths& paths = {});

        std::string combineSourceAndDefines(const std::string& source, const std::vector<std::string>& defines);
        std::string insertIncludes(const std::string& source, const Paths& paths);
        std::string readShaderSource(const Path& filename, const Paths& paths);

        void apply(Node& node) override;
        void apply(StateGroup& stategroup) override;
        void apply(BindGraphicsPipeline& bgp) override;
        void apply(BindComputePipeline& bgp) override;
        void apply(BindRayTracingPipeline& bgp) override;

    };
}
