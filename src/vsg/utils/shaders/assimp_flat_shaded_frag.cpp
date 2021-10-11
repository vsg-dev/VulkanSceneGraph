#include <vsg/io/VSG.h>
static auto assimp_flat_shaded_frag = []() {std::istringstream str(
R"(#vsga 0.1.7
Root id=1 vsg::ShaderStage
{
  NumUserObjects 0
  stage 16
  entryPointName "main"
  module id=2 vsg::ShaderModule
  {
    NumUserObjects 0
    Source "#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma import_defines (VSG_POINT_SPRITE, VSG_DIFFUSE_MAP, VSG_GREYSACLE_DIFFUSE_MAP)

#ifdef VSG_DIFFUSE_MAP
layout(binding = 0) uniform sampler2D diffuseMap;
#endif

layout(location = 2) in vec4 vertexColor;

#ifndef VSG_POINT_SPRITE
layout(location = 3) in vec2 texCoord0;
#endif

layout(binding = 10) uniform MaterialData
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    vec4 emissiveColor;
    float shininess;
    float alphaMask;
    float alphaMaskCutoff;
} material;

layout(location = 0) out vec4 outColor;

void main()
{
#ifdef VSG_POINT_SPRITE
    vec2 texCoord0 = gl_PointCoord.xy;
#endif

    vec4 diffuseColor = vertexColor * material.diffuseColor;

#ifdef VSG_DIFFUSE_MAP
    #ifdef VSG_GREYSACLE_DIFFUSE_MAP
        float v = texture(diffuseMap, texCoord0.st).s;
        diffuseColor *= vec4(v, v, v, 1.0);
    #else
        diffuseColor *= texture(diffuseMap, texCoord0.st);
    #endif
#endif

    if (material.alphaMask == 1.0f)
    {
        if (diffuseColor.a < material.alphaMaskCutoff)
            discard;
    }

    outColor = diffuseColor;
}
"
    hints id=0
    SPIRVSize 0
    SPIRV
  }
  NumSpecializationConstants 0
}
)");
vsg::VSG io;
return io.read_cast<vsg::ShaderStage>(str);
};
