#pragma once

#include <vsg/core/Visitor.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/state/ShaderStage.h>

namespace vsg
{

    /// ShaderCompiler integrates with GLSLang to provide shader compilation from GLSL shaders to SPIRV shaders usable by Vulkan.
    /// To be able to compile GLSL the VulkanSceneGraph has to be compiled against GLSLang, you can check whether shader compilation
    /// is supported via the VSG_SUPPORTS_ShaderCompiler #define provided in include/core/Version.h, if the value is 1 then shader compilation
    /// is supported. You can also check the ShaderCompiler::supported() method.
    class VSG_DECLSPEC ShaderCompiler : public Inherit<Visitor, ShaderCompiler>
    {
    public:
        ShaderCompiler();
        virtual ~ShaderCompiler();

        /// return true if shader compilation is supported by this build of VulkanSceneGraph
        /// you can also use the VSG_SUPPORTS_ShaderCompiler define provided by include/vsg/core/Version.h
        bool supported() const;

        // default ShaderCompileSettings
        ref_ptr<ShaderCompileSettings> defaults;

        bool compile(ShaderStages& shaders, const std::vector<std::string>& defines = {}, ref_ptr<const Options> options = {});
        bool compile(ref_ptr<ShaderStage> shaderStage, const std::vector<std::string>& defines = {}, ref_ptr<const Options> options = {});

        std::string combineSourceAndDefines(const std::string& source, const std::vector<std::string>& defines);

        void apply(Node& node) override;
        void apply(BindGraphicsPipeline& bgp) override;
        void apply(BindComputePipeline& bgp) override;
        void apply(BindRayTracingPipeline& bgp) override;

    protected:
        bool _initialized = false;
    };
    VSG_type_name(vsg::ShaderCompiler);

} // namespace vsg
