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
    SPIRVSize 386
    SPIRV 119734787 65536 524298 49 0 131089 1 393227 1 1280527431 1685353262 808793134
     0 196622 0 1 524303 4 4 1852399981 0 11 44 48
     196624 4 7 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331
     1868526181 1667590754 29556 262149 4 1852399981 0 393221 9 1717987684 1130722165 1919904879
     0 327685 11 1953654134 1866692709 7499628 393221 13 1702125901 1818323314 1635017028 0
     458758 13 0 1768058209 1131703909 1919904879 0 458758 13 1 1717987684 1130722165
     1919904879 0 458758 13 2 1667592307 1918987381 1869377347 114 458758 13 3
     1936289125 1702259059 1869377347 114 393222 13 4 1852401779 1936027241 115 393222 13
     5 1752198241 1935756641 107 458758 13 6 1752198241 1935756641 1953842027 6710895 327685
     15 1702125933 1818323314 0 327685 44 1131705711 1919904879 0 327685 48 1131963764
     1685221231 48 262215 11 30 2 327752 13 0 35 0 327752
     13 1 35 16 327752 13 2 35 32 327752 13 3
     35 48 327752 13 4 35 64 327752 13 5 35 68
     327752 13 6 35 72 196679 13 2 262215 15 34 0
     262215 15 33 10 262215 44 30 0 262215 48 30 3
     131091 2 196641 3 2 196630 6 32 262167 7 6 4
     262176 8 7 7 262176 10 1 7 262203 10 11 1
     589854 13 7 7 7 7 6 6 6 262176 14 2
     13 262203 14 15 2 262165 16 32 1 262187 16 17
     1 262176 18 2 7 262187 16 22 5 262176 23 2
     6 262187 6 26 1065353216 131092 27 262165 31 32 0 262187
     31 32 3 262176 33 7 6 262187 16 36 6 262176
     43 3 7 262203 43 44 3 262167 46 6 2 262176
     47 1 46 262203 47 48 1 327734 2 4 0 3
     131320 5 262203 8 9 7 262205 7 12 11 327745 18
     19 15 17 262205 7 20 19 327813 7 21 12 20
     196670 9 21 327745 23 24 15 22 262205 6 25 24
     327860 27 28 25 26 196855 30 0 262394 28 29 30
     131320 29 327745 33 34 9 32 262205 6 35 34 327745
     23 37 15 36 262205 6 38 37 327864 27 39 35
     38 196855 41 0 262394 39 40 41 131320 40 65788 131320
     41 131321 30 131320 30 262205 7 45 9 196670 44 45
     65789 65592
  }
  NumSpecializationConstants 0
}
)");
vsg::VSG io;
return io.read_cast<vsg::ShaderStage>(str);
};
