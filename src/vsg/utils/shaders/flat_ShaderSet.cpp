#include <vsg/io/VSG.h>
static auto flat_ShaderSet = []() {std::istringstream str(
R"(#vsga 0.6.0
Root id=1 vsg::ShaderSet
{
  userObjects 0
  stages 2
  vsg::ShaderStage id=2 vsg::ShaderStage
  {
    userObjects 0
    stage 1
    entryPointName "main"
    module id=3 vsg::ShaderModule
    {
      userObjects 0
      hints id=0
      source "#version 450
#extension GL_ARB_separate_shader_objects : enable

#pragma import_defines (VSG_INSTANCE_POSITIONS, VSG_DISPLACEMENT_MAP)

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;

#ifdef VSG_DISPLACEMENT_MAP
layout(binding = 6) uniform sampler2D displacementMap;
#endif

layout(location = 0) in vec3 vsg_Vertex;
layout(location = 1) in vec3 vsg_Normal;
layout(location = 2) in vec2 vsg_TexCoord0;
layout(location = 3) in vec4 vsg_Color;

#ifdef VSG_INSTANCE_POSITIONS
layout(location = 4) in vec3 vsg_position;
#endif

layout(location = 0) out vec3 eyePos;
layout(location = 1) out vec3 normalDir;
layout(location = 2) out vec4 vertexColor;
layout(location = 3) out vec2 texCoord0;

layout(location = 5) out vec3 viewDir;

out gl_PerVertex{ vec4 gl_Position; };

void main()
{
    vec4 vertex = vec4(vsg_Vertex, 1.0);
    vec4 normal = vec4(vsg_Normal, 0.0);

#ifdef VSG_DISPLACEMENT_MAP
    // TODO need to pass as as uniform or per instance attributes
    vec3 scale = vec3(1.0, 1.0, 1.0);

    vertex.xyz = vertex.xyz + vsg_Normal * (texture(displacementMap, vsg_TexCoord0.st).s * scale.z);

    float s_delta = 0.01;
    float width = 0.0;

    float s_left = max(vsg_TexCoord0.s - s_delta, 0.0);
    float s_right = min(vsg_TexCoord0.s + s_delta, 1.0);
    float t_center = vsg_TexCoord0.t;
    float delta_left_right = (s_right - s_left) * scale.x;
    float dz_left_right = (texture(displacementMap, vec2(s_right, t_center)).s - texture(displacementMap, vec2(s_left, t_center)).s) * scale.z;

    // TODO need to handle different origins of displacementMap vs diffuseMap etc,
    float t_delta = s_delta;
    float t_bottom = max(vsg_TexCoord0.t - t_delta, 0.0);
    float t_top = min(vsg_TexCoord0.t + t_delta, 1.0);
    float s_center = vsg_TexCoord0.s;
    float delta_bottom_top = (t_top - t_bottom) * scale.y;
    float dz_bottom_top = (texture(displacementMap, vec2(s_center, t_top)).s - texture(displacementMap, vec2(s_center, t_bottom)).s) * scale.z;

    vec3 dx = normalize(vec3(delta_left_right, 0.0, dz_left_right));
    vec3 dy = normalize(vec3(0.0, delta_bottom_top, -dz_bottom_top));
    vec3 dz = normalize(cross(dx, dy));

    normal.xyz = normalize(dx * vsg_Normal.x + dy * vsg_Normal.y + dz * vsg_Normal.z);
#endif


#ifdef VSG_INSTANCE_POSITIONS
   vertex.xyz = vertex.xyz + vsg_position;
#endif

    gl_Position = (pc.projection * pc.modelView) * vertex;

    eyePos = (pc.modelView * vertex).xyz;

    vec4 lpos = /*vsg_LightSource.position*/ vec4(0.0, 0.0, 1.0, 0.0);
    viewDir = - (pc.modelView * vertex).xyz;
    normalDir = (pc.modelView * normal).xyz;

    vertexColor = vsg_Color;
    texCoord0 = vsg_TexCoord0;
}
"
      code 0
      
    }
    NumSpecializationConstants 0
  }
  vsg::ShaderStage id=4 vsg::ShaderStage
  {
    userObjects 0
    stage 16
    entryPointName "main"
    module id=5 vsg::ShaderModule
    {
      userObjects 0
      hints id=0
      source "#version 450
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
      code 0
      
    }
    NumSpecializationConstants 0
  }
  attributeBindings 5
  name "vsg_Vertex"
  define ""
  location 0
  format 106
  data id=6 vsg::vec3Array
  {
    userObjects 0
    Layout 0 12 0 1 1 1 0 -1 0
    size 1
    storage id=0
    data 0 0 0
  }
  name "vsg_Normal"
  define ""
  location 1
  format 106
  data id=7 vsg::vec3Array
  {
    userObjects 0
    Layout 0 12 0 1 1 1 0 -1 0
    size 1
    storage id=0
    data 0 0 0
  }
  name "vsg_TexCoord0"
  define ""
  location 2
  format 103
  data id=8 vsg::vec2Array
  {
    userObjects 0
    Layout 0 8 0 1 1 1 0 -1 0
    size 1
    storage id=0
    data 0 0
  }
  name "vsg_Color"
  define ""
  location 3
  format 109
  data id=9 vsg::vec4Array
  {
    userObjects 0
    Layout 0 16 0 1 1 1 0 -1 0
    size 1
    storage id=0
    data 0 0 0 0
  }
  name "vsg_position"
  define "VSG_INSTANCE_POSITIONS"
  location 4
  format 106
  data id=10 vsg::vec3Array
  {
    userObjects 0
    Layout 0 12 0 1 1 1 0 -1 0
    size 1
    storage id=0
    data 0 0 0
  }
  uniformBindings 3
  name "displacementMap"
  define "VSG_DISPLACEMENT_MAP"
  set 0
  binding 6
  descriptorType 1
  descriptorCount 1
  stageFlags 1
  data id=11 vsg::vec4Array2D
  {
    userObjects 0
    Layout 0 16 0 1 1 1 0 -1 0
    width 1
    height 1
    storage id=0
    data 0 0 0 0
  }
  name "diffuseMap"
  define "VSG_DIFFUSE_MAP"
  set 0
  binding 0
  descriptorType 1
  descriptorCount 1
  stageFlags 16
  data id=12 vsg::vec4Array2D
  {
    userObjects 0
    Layout 0 16 0 1 1 1 0 -1 0
    width 1
    height 1
    storage id=0
    data 0 0 0 0
  }
  name "material"
  define ""
  set 0
  binding 10
  descriptorType 6
  descriptorCount 1
  stageFlags 16
  data id=13 vsg::PhongMaterialValue
  {
    userObjects 0
    Layout 0 0 0 1 1 1 0 -1 0
    Value    ambient 1 1 1 0.9
    diffuse 1 1 1 0.9
    specular 0 0 0 0.2
    emissive 0 0 0 0
    shininess 100
    alphaMask 1
    alphaMaskCutoff 0.5

  }
  pushConstantRanges 1
  name "pc"
  define ""
  stageFlags 1
  offset 0
  size 128
  definesArrayStates 3
  defines 2
  element "VSG_DISPLACEMENT_MAP"
  element "VSG_INSTANCE_POSITIONS"
  arrayState id=14 vsg::PositionAndDisplacementMapArrayState
  {
    userObjects 0
  }
  defines 1
  element "VSG_INSTANCE_POSITIONS"
  arrayState id=15 vsg::PositionArrayState
  {
    userObjects 0
  }
  defines 1
  element "VSG_DISPLACEMENT_MAP"
  arrayState id=16 vsg::DisplacementMapArrayState
  {
    userObjects 0
  }
  optionalDefines 2
  element "VSG_GREYSACLE_DIFFUSE_MAP"
  element "VSG_POINT_SPRITE"
  defaultGraphicsPipelineStates 0
  variants 2
  hints id=17 vsg::ShaderCompileSettings
  {
    vulkanVersion 4194304
    clientInputVersion 100
    language 0
    defaultVersion 450
    target 65536
    forwardCompatible 0
    defines 0
  }
  stages 2
  vsg::ShaderStage id=18 vsg::ShaderStage
  {
    userObjects 0
    stage 1
    entryPointName "main"
    module id=19 vsg::ShaderModule
    {
      userObjects 0
      hints id=17
      source ""
      code 583
       119734787 65536 524298 79 0 131089 1 393227 1 1280527431 1685353262 808793134
       0 196622 0 1 983055 0 4 1852399981 0 12 20 29
       48 56 63 69 71 75 77 196611 2 450 589828 1096764487
       1935622738 1918988389 1600484449 1684105331 1868526181 1667590754 29556 262149 4 1852399981 0 262149
       9 1953654134 30821 327685 12 1600615286 1953654102 30821 262149 19 1836216174 27745
       327685 20 1600615286 1836216142 27745 393221 27 1348430951 1700164197 2019914866 0 393222
       27 0 1348430951 1953067887 7237481 196613 29 0 393221 33 1752397136 1936617283
       1953390964 115 393222 33 0 1785688688 1769235301 28271 393222 33 1 1701080941
       1701402220 119 196613 35 25456 262149 48 1348827493 29551 262149 54 1936683116
       0 262149 56 2003134838 7498052 327685 63 1836216174 1766091873 114 327685 69
       1953654134 1866692709 7499628 327685 71 1600615286 1869377347 114 327685 75 1131963764 1685221231
       48 393221 77 1600615286 1131963732 1685221231 48 262215 12 30 0 262215
       20 30 1 327752 27 0 11 0 196679 27 2 262216
       33 0 5 327752 33 0 35 0 327752 33 0 7
       16 262216 33 1 5 327752 33 1 35 64 327752 33
       1 7 16 196679 33 2 262215 48 30 0 262215 56
       30 5 262215 63 30 1 262215 69 30 2 262215 71
       30 3 262215 75 30 3 262215 77 30 2 131091 2
       196641 3 2 196630 6 32 262167 7 6 4 262176 8
       7 7 262167 10 6 3 262176 11 1 10 262203 11
       12 1 262187 6 14 1065353216 262203 11 20 1 262187 6
       22 0 196638 27 7 262176 28 3 27 262203 28 29
       3 262165 30 32 1 262187 30 31 0 262168 32 7
       4 262174 33 32 32 262176 34 9 33 262203 34 35
       9 262176 36 9 32 262187 30 39 1 262176 45 3
       7 262176 47 3 10 262203 47 48 3 458796 7 55
       22 22 14 22 262203 47 56 3 262203 47 63 3
       262203 45 69 3 262176 70 1 7 262203 70 71 1
       262167 73 6 2 262176 74 3 73 262203 74 75 3
       262176 76 1 73 262203 76 77 1 327734 2 4 0
       3 131320 5 262203 8 9 7 262203 8 19 7 262203
       8 54 7 262205 10 13 12 327761 6 15 13 0
       327761 6 16 13 1 327761 6 17 13 2 458832 7
       18 15 16 17 14 196670 9 18 262205 10 21 20
       327761 6 23 21 0 327761 6 24 21 1 327761 6
       25 21 2 458832 7 26 23 24 25 22 196670 19
       26 327745 36 37 35 31 262205 32 38 37 327745 36
       40 35 39 262205 32 41 40 327826 32 42 38 41
       262205 7 43 9 327825 7 44 42 43 327745 45 46
       29 31 196670 46 44 327745 36 49 35 39 262205 32
       50 49 262205 7 51 9 327825 7 52 50 51 524367
       10 53 52 52 0 1 2 196670 48 53 196670 54
       55 327745 36 57 35 39 262205 32 58 57 262205 7
       59 9 327825 7 60 58 59 524367 10 61 60 60
       0 1 2 262271 10 62 61 196670 56 62 327745 36
       64 35 39 262205 32 65 64 262205 7 66 19 327825
       7 67 65 66 524367 10 68 67 67 0 1 2
       196670 63 68 262205 7 72 71 196670 69 72 262205 73
       78 77 196670 75 78 65789 65592
    }
    NumSpecializationConstants 0
  }
  vsg::ShaderStage id=20 vsg::ShaderStage
  {
    userObjects 0
    stage 16
    entryPointName "main"
    module id=21 vsg::ShaderModule
    {
      userObjects 0
      hints id=17
      source ""
      code 386
       119734787 65536 524298 49 0 131089 1 393227 1 1280527431 1685353262 808793134
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
  hints id=22 vsg::ShaderCompileSettings
  {
    vulkanVersion 4194304
    clientInputVersion 100
    language 0
    defaultVersion 450
    target 65536
    forwardCompatible 0
    defines 1
    element "VSG_DIFFUSE_MAP"
  }
  stages 2
  vsg::ShaderStage id=18
  vsg::ShaderStage id=23 vsg::ShaderStage
  {
    userObjects 0
    stage 16
    entryPointName "main"
    module id=24 vsg::ShaderModule
    {
      userObjects 0
      hints id=22
      source ""
      code 444
       119734787 65536 524298 58 0 131089 1 393227 1 1280527431 1685353262 808793134
       0 196622 0 1 524303 4 4 1852399981 0 11 29 56
       196624 4 7 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331
       1868526181 1667590754 29556 262149 4 1852399981 0 393221 9 1717987684 1130722165 1919904879
       0 327685 11 1953654134 1866692709 7499628 393221 13 1702125901 1818323314 1635017028 0
       458758 13 0 1768058209 1131703909 1919904879 0 458758 13 1 1717987684 1130722165
       1919904879 0 458758 13 2 1667592307 1918987381 1869377347 114 458758 13 3
       1936289125 1702259059 1869377347 114 393222 13 4 1852401779 1936027241 115 393222 13
       5 1752198241 1935756641 107 458758 13 6 1752198241 1935756641 1953842027 6710895 327685
       15 1702125933 1818323314 0 327685 25 1717987684 1298494325 28769 327685 29 1131963764
       1685221231 48 327685 56 1131705711 1919904879 0 262215 11 30 2 327752
       13 0 35 0 327752 13 1 35 16 327752 13 2
       35 32 327752 13 3 35 48 327752 13 4 35 64
       327752 13 5 35 68 327752 13 6 35 72 196679 13
       2 262215 15 34 0 262215 15 33 10 262215 25 34
       0 262215 25 33 0 262215 29 30 3 262215 56 30
       0 131091 2 196641 3 2 196630 6 32 262167 7 6
       4 262176 8 7 7 262176 10 1 7 262203 10 11
       1 589854 13 7 7 7 7 6 6 6 262176 14
       2 13 262203 14 15 2 262165 16 32 1 262187 16
       17 1 262176 18 2 7 589849 22 6 1 0 0
       0 1 0 196635 23 22 262176 24 0 23 262203 24
       25 0 262167 27 6 2 262176 28 1 27 262203 28
       29 1 262187 16 34 5 262176 35 2 6 262187 6
       38 1065353216 131092 39 262165 43 32 0 262187 43 44 3
       262176 45 7 6 262187 16 48 6 262176 55 3 7
       262203 55 56 3 327734 2 4 0 3 131320 5 262203
       8 9 7 262205 7 12 11 327745 18 19 15 17
       262205 7 20 19 327813 7 21 12 20 196670 9 21
       262205 23 26 25 262205 27 30 29 327767 7 31 26
       30 262205 7 32 9 327813 7 33 32 31 196670 9
       33 327745 35 36 15 34 262205 6 37 36 327860 39
       40 37 38 196855 42 0 262394 40 41 42 131320 41
       327745 45 46 9 44 262205 6 47 46 327745 35 49
       15 48 262205 6 50 49 327864 39 51 47 50 196855
       53 0 262394 51 52 53 131320 52 65788 131320 53 131321
       42 131320 42 262205 7 57 9 196670 56 57 65789 65592
    }
    NumSpecializationConstants 0
  }
}
)");
vsg::VSG io;
return io.read_cast<vsg::ShaderSet>(str);
};
