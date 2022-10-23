#include <vsg/io/VSG.h>
static auto phong_ShaderSet = []() {std::istringstream str(
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
#pragma import_defines (VSG_POINT_SPRITE, VSG_DIFFUSE_MAP, VSG_GREYSACLE_DIFFUSE_MAP, VSG_EMISSIVE_MAP, VSG_LIGHTMAP_MAP, VSG_NORMAL_MAP, VSG_SPECULAR_MAP, VSG_TWO_SIDED_LIGHTING)

#ifdef VSG_DIFFUSE_MAP
layout(set = 0, binding = 0) uniform sampler2D diffuseMap;
#endif

#ifdef VSG_NORMAL_MAP
layout(set = 0, binding = 2) uniform sampler2D normalMap;
#endif

#ifdef VSG_LIGHTMAP_MAP
layout(set = 0, binding = 3) uniform sampler2D aoMap;
#endif

#ifdef VSG_EMISSIVE_MAP
layout(set = 0, binding = 4) uniform sampler2D emissiveMap;
#endif

#ifdef VSG_SPECULAR_MAP
layout(set = 0, binding = 5) uniform sampler2D specularMap;
#endif

layout(set = 0, binding = 10) uniform MaterialData
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    vec4 emissiveColor;
    float shininess;
    float alphaMask;
    float alphaMaskCutoff;
} material;

layout(set = 1, binding = 0) uniform LightData
{
    vec4 values[64];
} lightData;

layout(location = 0) in vec3 eyePos;
layout(location = 1) in vec3 normalDir;
layout(location = 2) in vec4 vertexColor;
#ifndef VSG_POINT_SPRITE
layout(location = 3) in vec2 texCoord0;
#endif
layout(location = 5) in vec3 viewDir;

layout(location = 0) out vec4 outColor;

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal()
{
#ifdef VSG_NORMAL_MAP
    // Perturb normal, see http://www.thetenthplanet.de/archives/1180
    vec3 tangentNormal = texture(normalMap, texCoord0).xyz * 2.0 - 1.0;

    //tangentNormal *= vec3(2,2,1);

    vec3 q1 = dFdx(eyePos);
    vec3 q2 = dFdy(eyePos);
    vec2 st1 = dFdx(texCoord0);
    vec2 st2 = dFdy(texCoord0);

    vec3 N = normalize(normalDir);
    vec3 T = normalize(q1 * st2.t - q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
#else
    return normalize(normalDir);
#endif
}

vec3 computeLighting(vec3 ambientColor, vec3 diffuseColor, vec3 specularColor, vec3 emissiveColor, float shininess, float ambientOcclusion, vec3 ld, vec3 nd, vec3 vd)
{
    vec3 color = vec3(0.0);
    color.rgb += ambientColor;

    float diff = max(dot(ld, nd), 0.0);
    color.rgb += diffuseColor * diff;

    if (diff > 0.0)
    {
        vec3 halfDir = normalize(ld + vd);
        color.rgb += specularColor * pow(max(dot(halfDir, nd), 0.0), shininess);
    }

    vec3 result = color + emissiveColor;
    result *= ambientOcclusion;

    return result;
}

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

    vec4 ambientColor = diffuseColor * material.ambientColor * material.ambientColor.a;
    vec4 specularColor = material.specularColor;
    vec4 emissiveColor = material.emissiveColor;
    float shininess = material.shininess;
    float ambientOcclusion = 1.0;

    if (material.alphaMask == 1.0f)
    {
        if (diffuseColor.a < material.alphaMaskCutoff)
            discard;
    }

#ifdef VSG_EMISSIVE_MAP
    emissiveColor *= texture(emissiveMap, texCoord0.st);
#endif

#ifdef VSG_LIGHTMAP_MAP
    ambientOcclusion *= texture(aoMap, texCoord0.st).r;
#endif

#ifdef VSG_SPECULAR_MAP
    specularColor *= texture(specularMap, texCoord0.st);
#endif

    vec3 nd = getNormal();
    vec3 vd = normalize(viewDir);

    vec3 color = vec3(0.0, 0.0, 0.0);

    vec4 lightNums = lightData.values[0];
    int numAmbientLights = int(lightNums[0]);
    int numDirectionalLights = int(lightNums[1]);
    int numPointLights = int(lightNums[2]);
    int numSpotLights = int(lightNums[3]);
    int index = 1;

    if (numAmbientLights>0)
    {
        // ambient lights
        for(int i = 0; i<numAmbientLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            color += (ambientColor.rgb * lightColor.rgb) * (lightColor.a);
        }
    }

    if (numDirectionalLights>0)
    {
        // directional lights
        for(int i = 0; i<numDirectionalLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            vec3 direction = -lightData.values[index++].xyz;

            float unclamped_LdotN = dot(direction, nd);
            #ifdef VSG_TWO_SIDED_LIGHTING
            if (unclamped_LdotN < 0.0)
            {
                nd = -nd;
                unclamped_LdotN = -unclamped_LdotN;
            }
            #endif

            float diff = max(unclamped_LdotN, 0.0);
            color.rgb += (diffuseColor.rgb * lightColor.rgb) * (diff * lightColor.a);

            if (shininess > 0.0 && diff > 0.0)
            {
                vec3 halfDir = normalize(direction + vd);
                color.rgb += specularColor.rgb * (pow(max(dot(halfDir, nd), 0.0), shininess) * lightColor.a);
            }
        }
    }

    if (numPointLights>0)
    {
        // point light
        for(int i = 0; i<numPointLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            vec3 position = lightData.values[index++].xyz;
            vec3 delta = position - eyePos;
            float distance2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            vec3 direction = delta / sqrt(distance2);
            float scale = lightColor.a / distance2;

            float unclamped_LdotN = dot(direction, nd);
            #ifdef VSG_TWO_SIDED_LIGHTING
            if (unclamped_LdotN < 0.0)
            {
                nd = -nd;
                unclamped_LdotN = -unclamped_LdotN;
            }
            #endif

            float diff = scale * max(unclamped_LdotN, 0.0);

            color.rgb += (diffuseColor.rgb * lightColor.rgb) * diff;
            if (shininess > 0.0 && diff > 0.0)
            {
                vec3 halfDir = normalize(direction + vd);
                color.rgb += specularColor.rgb * (pow(max(dot(halfDir, nd), 0.0), shininess) * scale);
            }
        }
    }

    if (numSpotLights>0)
    {
        // spot light
        for(int i = 0; i<numSpotLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            vec4 position_cosInnerAngle = lightData.values[index++];
            vec4 lightDirection_cosOuterAngle = lightData.values[index++];

            vec3 delta = position_cosInnerAngle.xyz - eyePos;
            float distance2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            vec3 direction = delta / sqrt(distance2);

            float dot_lightdirection = dot(lightDirection_cosOuterAngle.xyz, -direction);
            float scale = (lightColor.a  * smoothstep(lightDirection_cosOuterAngle.w, position_cosInnerAngle.w, dot_lightdirection)) / distance2;

            float unclamped_LdotN = dot(direction, nd);
            #ifdef VSG_TWO_SIDED_LIGHTING
            if (unclamped_LdotN < 0.0)
            {
                nd = -nd;
                unclamped_LdotN = -unclamped_LdotN;
            }
            #endif

            float diff = scale * max(unclamped_LdotN, 0.0);
            color.rgb += (diffuseColor.rgb * lightColor.rgb) * diff;
            if (shininess > 0.0 && diff > 0.0)
            {
                vec3 halfDir = normalize(direction + vd);
                color.rgb += specularColor.rgb * (pow(max(dot(halfDir, nd), 0.0), shininess) * scale);
            }
        }
    }

    outColor.rgb = (color * ambientOcclusion) + emissiveColor.rgb;
    outColor.a = diffuseColor.a;
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
  uniformBindings 7
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
  name "normalMap"
  define "VSG_NORMAL_MAP"
  set 0
  binding 2
  descriptorType 1
  descriptorCount 1
  stageFlags 16
  data id=13 vsg::vec3Array2D
  {
    userObjects 0
    Layout 0 12 0 1 1 1 0 -1 0
    width 1
    height 1
    storage id=0
    data 0 0 0
  }
  name "aoMap"
  define "VSG_LIGHTMAP_MAP"
  set 0
  binding 3
  descriptorType 1
  descriptorCount 1
  stageFlags 16
  data id=14 vsg::vec4Array2D
  {
    userObjects 0
    Layout 0 16 0 1 1 1 0 -1 0
    width 1
    height 1
    storage id=0
    data 0 0 0 0
  }
  name "emissiveMap"
  define "VSG_EMISSIVE_MAP"
  set 0
  binding 4
  descriptorType 1
  descriptorCount 1
  stageFlags 16
  data id=15 vsg::vec4Array2D
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
  data id=16 vsg::PhongMaterialValue
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
  name "lightData"
  define ""
  set 1
  binding 0
  descriptorType 6
  descriptorCount 1
  stageFlags 16
  data id=17 vsg::vec4Array
  {
    userObjects 0
    Layout 0 16 0 1 1 1 0 -1 0
    size 64
    storage id=0
    data 0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0 0 0 0 0 0 0 0 0
     0 0 0 0
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
  arrayState id=18 vsg::PositionAndDisplacementMapArrayState
  {
    userObjects 0
  }
  defines 1
  element "VSG_INSTANCE_POSITIONS"
  arrayState id=19 vsg::PositionArrayState
  {
    userObjects 0
  }
  defines 1
  element "VSG_DISPLACEMENT_MAP"
  arrayState id=20 vsg::DisplacementMapArrayState
  {
    userObjects 0
  }
  optionalDefines 3
  element "VSG_GREYSACLE_DIFFUSE_MAP"
  element "VSG_POINT_SPRITE"
  element "VSG_TWO_SIDED_LIGHTING"
  defaultGraphicsPipelineStates 0
  variants 2
  hints id=21 vsg::ShaderCompileSettings
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
  vsg::ShaderStage id=22 vsg::ShaderStage
  {
    userObjects 0
    stage 1
    entryPointName "main"
    module id=23 vsg::ShaderModule
    {
      userObjects 0
      hints id=21
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
)"
R"(       1701402220 119 196613 35 25456 262149 48 1348827493 29551 262149 54 1936683116
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
  vsg::ShaderStage id=24 vsg::ShaderStage
  {
    userObjects 0
    stage 16
    entryPointName "main"
    module id=25 vsg::ShaderModule
    {
      userObjects 0
      hints id=21
      source ""
      code 2822
       119734787 65536 524298 460 0 131089 1 393227 1 1280527431 1685353262 808793134
       0 196622 0 1 720911 4 4 1852399981 0 12 21 79
       242 440 459 196624 4 7 196611 2 450 589828 1096764487 1935622738
       1918988389 1600484449 1684105331 1868526181 1667590754 29556 262149 4 1852399981 0 327685 9
       1316250983 1634562671 10348 327685 12 1836216174 1766091873 114 393221 19 1717987684 1130722165
       1919904879 0 327685 21 1953654134 1866692709 7499628 393221 23 1702125901 1818323314 1635017028
       0 458758 23 0 1768058209 1131703909 1919904879 0 458758 23 1 1717987684
       1130722165 1919904879 0 458758 23 2 1667592307 1918987381 1869377347 114 458758 23
       3 1936289125 1702259059 1869377347 114 393222 23 4 1852401779 1936027241 115 393222
       23 5 1752198241 1935756641 107 458758 23 6 1752198241 1935756641 1953842027 6710895
       327685 25 1702125933 1818323314 0 393221 32 1768058209 1131703909 1919904879 0 393221
       44 1667592307 1918987381 1869377347 114 393221 48 1936289125 1702259059 1869377347 114 327685
       53 1852401779 1936027241 115 458757 57 1768058209 1333030501 1970037603 1852795251 0 196613
       76 25710 196613 78 25718 262149 79 2003134838 7498052 262149 82 1869377379
       114 327685 85 1751607660 1836404340 115 327685 88 1751607628 1952531572 97 327686
       88 0 1970037110 29541 327685 90 1751607660 1952531572 97 458757 94 1097692526
       1701405293 1766618222 1937008743 0 524293 99 1148024174 1667592809 1852795252 1766616161 1937008743 0
       393221 104 1349350766 1953393007 1751607628 29556 393221 109 1399682414 1282699120 1952999273 115
       262149 113 1701080681 120 196613 118 105 327685 127 1751607660 1819231092 29295
       196613 148 105 327685 157 1751607660 1819231092 29295 327685 162 1701996900 1869182051
       110 393221 169 1818455669 1701866849 1682726756 5141615 262149 173 1717987684 0 262149
       195 1718378856 7498052 196613 220 105 327685 229 1751607660 1819231092 29295 327685
       234 1769172848 1852795252 0 262149 240 1953260900 97 262149 242 1348827493 29551
       327685 245 1953720676 1701015137 50 327685 263 1701996900 1869182051 110 262149 269
       1818321779 101 393221 274 1818455669 1701866849 1682726756 5141615 262149 278 1717987684 0
       262149 299 1718378856 7498052 196613 323 105 327685 332 1751607660 1819231092 29295
       524293 337 1769172848 1852795252 1936679775 1701736009 1735278962 25964 655365 342 1751607660 1919501428
       1769235301 1667198575 1968141167 1098016116 1701603182 0 262149 347 1953260900 97 327685 352
       1953720676 1701015137 50 327685 370 1701996900 1869182051 110 458757 376 1601466212 1751607660
       1919509620 1769235301 28271 262149 382 1818321779 101 393221 394 1818455669 1701866849 1682726756
       5141615 262149 398 1717987684 0 262149 419 1718378856 7498052 327685 440 1131705711
       1919904879 0 327685 459 1131963764 1685221231 48 262215 12 30 1 262215
       21 30 2 327752 23 0 35 0 327752 23 1 35
       16 327752 23 2 35 32 327752 23 3 35 48 327752
       23 4 35 64 327752 23 5 35 68 327752 23 6
       35 72 196679 23 2 262215 25 34 0 262215 25 33
       10 262215 79 30 5 262215 87 6 16 327752 88 0
       35 0 196679 88 2 262215 90 34 1 262215 90 33
       0 262215 242 30 0 262215 440 30 0 262215 459 30
       3 131091 2 196641 3 2 196630 6 32 262167 7 6
       3 196641 8 7 262176 11 1 7 262203 11 12 1
       262167 17 6 4 262176 18 7 17 262176 20 1 17
       262203 20 21 1 589854 23 17 17 17 17 6 6
       6 262176 24 2 23 262203 24 25 2 262165 26 32
       1 262187 26 27 1 262176 28 2 17 262187 26 34
       0 262165 38 32 0 262187 38 39 3 262176 40 2
       6 262187 26 45 2 262187 26 49 3 262176 52 7
       6 262187 26 54 4 262187 6 58 1065353216 262187 26 59
       5 131092 62 262187 26 68 6 262176 75 7 7 262203
       11 79 1 262187 6 83 0 393260 7 84 83 83
       83 262187 38 86 64 262172 87 17 86 196638 88 87
       262176 89 2 88 262203 89 90 2 262176 93 7 26
       262187 38 95 0 262187 38 100 1 262187 38 105 2
       262203 11 242 1 262176 439 3 17 262203 439 440 3
       262176 447 3 6 262167 457 6 2 262176 458 1 457
       262203 458 459 1 327734 2 4 0 3 131320 5 262203
       18 19 7 262203 18 32 7 262203 18 44 7 262203
       18 48 7 262203 52 53 7 262203 52 57 7 262203
       75 76 7 262203 75 78 7 262203 75 82 7 262203
       18 85 7 262203 93 94 7 262203 93 99 7 262203
       93 104 7 262203 93 109 7 262203 93 113 7 262203
       93 118 7 262203 18 127 7 262203 93 148 7 262203
       18 157 7 262203 75 162 7 262203 52 169 7 262203
       52 173 7 262203 75 195 7 262203 93 220 7 262203
       18 229 7 262203 75 234 7 262203 75 240 7 262203
       52 245 7 262203 75 263 7 262203 52 269 7 262203
       52 274 7 262203 52 278 7 262203 75 299 7 262203
       93 323 7 262203 18 332 7 262203 18 337 7 262203
       18 342 7 262203 75 347 7 262203 52 352 7 262203
       75 370 7 262203 52 376 7 262203 52 382 7 262203
       52 394 7 262203 52 398 7 262203 75 419 7 262205
       17 22 21 327745 28 29 25 27 262205 17 30 29
       327813 17 31 22 30 196670 19 31 262205 17 33 19
       327745 28 35 25 34 262205 17 36 35 327813 17 37
       33 36 393281 40 41 25 34 39 262205 6 42 41
       327822 17 43 37 42 196670 32 43 327745 28 46 25
       45 262205 17 47 46 196670 44 47 327745 28 50 25
       49 262205 17 51 50 196670 48 51 327745 40 55 25
       54 262205 6 56 55 196670 53 56 196670 57 58 327745
       40 60 25 59 262205 6 61 60 327860 62 63 61
       58 196855 65 0 262394 63 64 65 131320 64 327745 52
       66 19 39 262205 6 67 66 327745 40 69 25 68
       262205 6 70 69 327864 62 71 67 70 196855 73 0
       262394 71 72 73 131320 72 65788 131320 73 131321 65 131320
       65 262201 7 77 9 196670 76 77 262205 7 80 79
       393228 7 81 1 69 80 196670 78 81 196670 82 84
       393281 28 91 90 34 34 262205 17 92 91 196670 85
       92 327745 52 96 85 95 262205 6 97 96 262254 26
       98 97 196670 94 98 327745 52 101 85 100 262205 6
       102 101 262254 26 103 102 196670 99 103 327745 52 106
       85 105 262205 6 107 106 262254 26 108 107 196670 104
       108 327745 52 110 85 39 262205 6 111 110 262254 26
       112 111 196670 109 112 196670 113 27 262205 26 114 94
       327853 62 115 114 34 196855 117 0 262394 115 116 117
       131320 116 196670 118 34 131321 119 131320 119 262390 121 122
       0 131321 123 131320 123 262205 26 124 118 262205 26 125
       94 327857 62 126 124 125 262394 126 120 121 131320 120
       262205 26 128 113 327808 26 129 128 27 196670 113 129
       393281 28 130 90 34 128 262205 17 131 130 196670 127
       131 262205 17 132 32 524367 7 133 132 132 0 1
       2 262205 17 134 127 524367 7 135 134 134 0 1
       2 327813 7 136 133 135 327745 52 137 127 39 262205
       6 138 137 327822 7 139 136 138 262205 7 140 82
       327809 7 141 140 139 196670 82 141 131321 122 131320 122
       262205 26 142 118 327808 26 143 142 27 196670 118 143
       131321 119 131320 121 131321 117 131320 117 262205 26 144 99
       327853 62 145 144 34 196855 147 0 262394 145 146 147
       131320 146 196670 148 34 131321 149 131320 149 262390 151 152
       0 131321 153 131320 153 262205 26 154 148 262205 26 155
       99 327857 62 156 154 155 262394 156 150 151 131320 150
       262205 26 158 113 327808 26 159 158 27 196670 113 159
       393281 28 160 90 34 158 262205 17 161 160 196670 157
       161 262205 26 163 113 327808 26 164 163 27 196670 113
       164 393281 28 165 90 34 163 262205 17 166 165 524367
       7 167 166 166 0 1 2 262271 7 168 167 196670
       162 168 262205 7 170 162 262205 7 171 76 327828 6
       172 170 171 196670 169 172 262205 6 174 169 458764 6
       175 1 40 174 83 196670 173 175 262205 17 176 19
       524367 7 177 176 176 0 1 2 262205 17 178 157
       524367 7 179 178 178 0 1 2 327813 7 180 177
       179 262205 6 181 173 327745 52 182 157 39 262205 6
       183 182 327813 6 184 181 183 327822 7 185 180 184
       262205 7 186 82 327809 7 187 186 185 196670 82 187
       262205 6 188 53 327866 62 189 188 83 262205 6 190
       173 327866 62 191 190 83 327847 62 192 189 191 196855
       194 0 262394 192 193 194 131320 193 262205 7 196 162
       262205 7 197 78 327809 7 198 196 197 393228 7 199
       1 69 198 196670 195 199 262205 17 200 44 524367 7
       201 200 200 0 1 2 262205 7 202 195 262205 7
       203 76 327828 6 204 202 203 458764 6 205 1 40
       204 83 262205 6 206 53 458764 6 207 1 26 205
       206 327745 52 208 157 39 262205 6 209 208 327813 6
       210 207 209 327822 7 211 201 210 262205 7 212 82
       327809 7 213 212 211 196670 82 213 131321 194 131320 194
       131321 152 131320 152 262205 26 214 148 327808 26 215 214
       27 196670 148 215 131321 149 131320 151 131321 147 131320 147
       262205 26 216 104 327853 62 217 216 34 196855 219 0
       262394 217 218 219 131320 218 196670 220 34 131321 221 131320
       221 262390 223 224 0 131321 225 131320 225 262205 26 226
       220 262205 26 227 104 327857 62 228 226 227 262394 228
       222 223 131320 222 262205 26 230 113 327808 26 231 230
       27 196670 113 231 393281 28 232 90 34 230 262205 17
       233 232 196670 229 233 262205 26 235 113 327808 26 236
       235 27 196670 113 236 393281 28 237 90 34 235 262205
       17 238 237 524367 7 239 238 238 0 1 2 196670
       234 239 262205 7 241 234 262205 7 243 242 327811 7
       244 241 243 196670 240 244 327745 52 246 240 95 262205
       6 247 246 327745 52 248 240 95 262205 6 249 248
       327813 6 250 247 249 327745 52 251 240 100 262205 6
       252 251 327745 52 253 240 100 262205 6 254 253 327813
       6 255 252 254 327809 6 256 250 255 327745 52 257
       240 105 262205 6 258 257 327745 52 259 240 105 262205
       6 260 259 327813 6 261 258 260 327809 6 262 256
       261 196670 245 262 262205 7 264 240 262205 6 265 245
       393228 6 266 1 31 265 393296 7 267 266 266 266
       327816 7 268 264 267 196670 263 268 327745 52 270 229
       39 262205 6 271 270 262205 6 272 245 327816 6 273
       271 272 196670 269 273 262205 7 275 263 262205 7 276
       76 327828 6 277 275 276 196670 274 277 262205 6 279
       269 262205 6 280 274 458764 6 281 1 40 280 83
       327813 6 282 279 281 196670 278 282 262205 17 283 19
       524367 7 284 283 283 0 1 2 262205 17 285 229
       524367 7 286 285 285 0 1 2 327813 7 287 284
       286 262205 6 288 278 327822 7 289 287 288 262205 7
       290 82 327809 7 291 290 289 196670 82 291 262205 6
       292 53 327866 62 293 292 83 262205 6 294 278 327866
       62 295 294 83 327847 62 296 293 295 196855 298 0
       262394 296 297 298 131320 297 262205 7 300 263 262205 7
       301 78 327809 7 302 300 301 393228 7 303 1 69
       302 196670 299 303 262205 17 304 44 524367 7 305 304
       304 0 1 2 262205 7 306 299 262205 7 307 76
       327828 6 308 306 307 458764 6 309 1 40 308 83
       262205 6 310 53 458764 6 311 1 26 309 310 262205
       6 312 269 327813 6 313 311 312 327822 7 314 305
       313 262205 7 315 82 327809 7 316 315 314 196670 82
       316 131321 298 131320 298 131321 224 131320 224 262205 26 317
       220 327808 26 318 317 27 196670 220 318 131321 221 131320
       223 131321 219 131320 219 262205 26 319 109 327853 62 320
       319 34 196855 322 0 262394 320 321 322 131320 321 196670
       323 34 131321 324 131320 324 262390 326 327 0 131321 328
       131320 328 262205 26 329 323 262205 26 330 109 327857 62
       331 329 330 262394 331 325 326 131320 325 262205 26 333
       113 327808 26 334 333 27 196670 113 334 393281 28 335
       90 34 333 262205 17 336 335 196670 332 336 262205 26
       338 113 327808 26 339 338 27 196670 113 339 393281 28
       340 90 34 338 262205 17 341 340 196670 337 341 262205
       26 343 113 327808 26 344 343 27 196670 113 344 393281
       28 345 90 34 343 262205 17 346 345 196670 342 346
       262205 17 348 337 524367 7 349 348 348 0 1 2
       262205 7 350 242 327811 7 351 349 350 196670 347 351
       327745 52 353 347 95 262205 6 354 353 327745 52 355
       347 95 262205 6 356 355 327813 6 357 354 356 327745
       52 358 347 100 262205 6 359 358 327745 52 360 347
       100 262205 6 361 360 327813 6 362 359 361 327809 6
       363 357 362 327745 52 364 347 105 262205 6 365 364
       327745 52 366 347 105 262205 6 367 366 327813 6 368
       365 367 327809 6 369 363 368 196670 352 369 262205 7
       371 347 262205 6 372 352 393228 6 373 1 31 372
       393296 7 374 373 373 373 327816 7 375 371 374 196670
       370 375 262205 17 377 342 524367 7 378 377 377 0
       1 2 262205 7 379 370 262271 7 380 379 327828 6
       381 378 380 196670 376 381 327745 52 383 332 39 262205
       6 384 383 327745 52 385 342 39 262205 6 386 385
       327745 52 387 337 39 262205 6 388 387 262205 6 389
       376 524300 6 390 1 49 386 388 389 327813 6 391
       384 390 262205 6 392 352 327816 6 393 391 392 196670
       382 393 262205 7 395 370 262205 7 396 76 327828 6
       397 395 396 196670 394 397 262205 6 399 382 262205 6
       400 394 458764 6 401 1 40 400 83 327813 6 402
       399 401 196670 398 402 262205 17 403 19 524367 7 404
       403 403 0 1 2 262205 17 405 332 524367 7 406
       405 405 0 1 2 327813 7 407 404 406 262205 6
       408 398 327822 7 409 407 408 262205 7 410 82 327809
       7 411 410 409 196670 82 411 262205 6 412 53 327866
       62 413 412 83 262205 6 414 398 327866 62 415 414
       83 327847 62 416 413 415 196855 418 0 262394 416 417
       418 131320 417 262205 7 420 370 262205 7 421 78 327809
       7 422 420 421 393228 7 423 1 69 422 196670 419
       423 262205 17 424 44 524367 7 425 424 424 0 1
       2 262205 7 426 419 262205 7 427 76 327828 6 428
       426 427 458764 6 429 1 40 428 83 262205 6 430
       53 458764 6 431 1 26 429 430 262205 6 432 382
       327813 6 433 431 432 327822 7 434 425 433 262205 7
)"
R"(       435 82 327809 7 436 435 434 196670 82 436 131321 418
       131320 418 131321 327 131320 327 262205 26 437 323 327808 26
       438 437 27 196670 323 438 131321 324 131320 326 131321 322
       131320 322 262205 7 441 82 262205 6 442 57 327822 7
       443 441 442 262205 17 444 48 524367 7 445 444 444
       0 1 2 327809 7 446 443 445 327745 447 448 440
       95 327761 6 449 446 0 196670 448 449 327745 447 450
       440 100 327761 6 451 446 1 196670 450 451 327745 447
       452 440 105 327761 6 453 446 2 196670 452 453 327745
       52 454 19 39 262205 6 455 454 327745 447 456 440
       39 196670 456 455 65789 65592 327734 7 9 0 8 131320
       10 262205 7 13 12 393228 7 14 1 69 13 131326
       14 65592
    }
    NumSpecializationConstants 0
  }
  hints id=26 vsg::ShaderCompileSettings
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
  vsg::ShaderStage id=22
  vsg::ShaderStage id=27 vsg::ShaderStage
  {
    userObjects 0
    stage 16
    entryPointName "main"
    module id=28 vsg::ShaderModule
    {
      userObjects 0
      hints id=26
      source ""
      code 2880
       119734787 65536 524298 469 0 131089 1 393227 1 1280527431 1685353262 808793134
       0 196622 0 1 720911 4 4 1852399981 0 12 21 39
       91 254 452 196624 4 7 196611 2 450 589828 1096764487 1935622738
       1918988389 1600484449 1684105331 1868526181 1667590754 29556 262149 4 1852399981 0 327685 9
       1316250983 1634562671 10348 327685 12 1836216174 1766091873 114 393221 19 1717987684 1130722165
       1919904879 0 327685 21 1953654134 1866692709 7499628 393221 23 1702125901 1818323314 1635017028
       0 458758 23 0 1768058209 1131703909 1919904879 0 458758 23 1 1717987684
       1130722165 1919904879 0 458758 23 2 1667592307 1918987381 1869377347 114 458758 23
       3 1936289125 1702259059 1869377347 114 393222 23 4 1852401779 1936027241 115 393222
       23 5 1752198241 1935756641 107 458758 23 6 1752198241 1935756641 1953842027 6710895
       327685 25 1702125933 1818323314 0 327685 35 1717987684 1298494325 28769 327685 39
       1131963764 1685221231 48 393221 44 1768058209 1131703909 1919904879 0 393221 56 1667592307
       1918987381 1869377347 114 393221 60 1936289125 1702259059 1869377347 114 327685 65 1852401779
       1936027241 115 458757 69 1768058209 1333030501 1970037603 1852795251 0 196613 88 25710
       196613 90 25718 262149 91 2003134838 7498052 262149 94 1869377379 114 327685
       97 1751607660 1836404340 115 327685 100 1751607628 1952531572 97 327686 100 0
       1970037110 29541 327685 102 1751607660 1952531572 97 458757 106 1097692526 1701405293 1766618222
       1937008743 0 524293 111 1148024174 1667592809 1852795252 1766616161 1937008743 0 393221 116
       1349350766 1953393007 1751607628 29556 393221 121 1399682414 1282699120 1952999273 115 262149 125
       1701080681 120 196613 130 105 327685 139 1751607660 1819231092 29295 196613 160
       105 327685 169 1751607660 1819231092 29295 327685 174 1701996900 1869182051 110 393221
       181 1818455669 1701866849 1682726756 5141615 262149 185 1717987684 0 262149 207 1718378856
       7498052 196613 232 105 327685 241 1751607660 1819231092 29295 327685 246 1769172848
       1852795252 0 262149 252 1953260900 97 262149 254 1348827493 29551 327685 257
       1953720676 1701015137 50 327685 275 1701996900 1869182051 110 262149 281 1818321779 101
       393221 286 1818455669 1701866849 1682726756 5141615 262149 290 1717987684 0 262149 311
       1718378856 7498052 196613 335 105 327685 344 1751607660 1819231092 29295 524293 349
       1769172848 1852795252 1936679775 1701736009 1735278962 25964 655365 354 1751607660 1919501428 1769235301 1667198575
       1968141167 1098016116 1701603182 0 262149 359 1953260900 97 327685 364 1953720676 1701015137
       50 327685 382 1701996900 1869182051 110 458757 388 1601466212 1751607660 1919509620 1769235301
       28271 262149 394 1818321779 101 393221 406 1818455669 1701866849 1682726756 5141615 262149
       410 1717987684 0 262149 431 1718378856 7498052 327685 452 1131705711 1919904879 0
       262215 12 30 1 262215 21 30 2 327752 23 0 35
       0 327752 23 1 35 16 327752 23 2 35 32 327752
       23 3 35 48 327752 23 4 35 64 327752 23 5
       35 68 327752 23 6 35 72 196679 23 2 262215 25
       34 0 262215 25 33 10 262215 35 34 0 262215 35
       33 0 262215 39 30 3 262215 91 30 5 262215 99
       6 16 327752 100 0 35 0 196679 100 2 262215 102
       34 1 262215 102 33 0 262215 254 30 0 262215 452
       30 0 131091 2 196641 3 2 196630 6 32 262167 7
       6 3 196641 8 7 262176 11 1 7 262203 11 12
       1 262167 17 6 4 262176 18 7 17 262176 20 1
       17 262203 20 21 1 589854 23 17 17 17 17 6
       6 6 262176 24 2 23 262203 24 25 2 262165 26
       32 1 262187 26 27 1 262176 28 2 17 589849 32
       6 1 0 0 0 1 0 196635 33 32 262176 34
       0 33 262203 34 35 0 262167 37 6 2 262176 38
       1 37 262203 38 39 1 262187 26 46 0 262165 50
       32 0 262187 50 51 3 262176 52 2 6 262187 26
       57 2 262187 26 61 3 262176 64 7 6 262187 26
       66 4 262187 6 70 1065353216 262187 26 71 5 131092 74
       262187 26 80 6 262176 87 7 7 262203 11 91 1
       262187 6 95 0 393260 7 96 95 95 95 262187 50
       98 64 262172 99 17 98 196638 100 99 262176 101 2
       100 262203 101 102 2 262176 105 7 26 262187 50 107
       0 262187 50 112 1 262187 50 117 2 262203 11 254
       1 262176 451 3 17 262203 451 452 3 262176 459 3
       6 327734 2 4 0 3 131320 5 262203 18 19 7
       262203 18 44 7 262203 18 56 7 262203 18 60 7
       262203 64 65 7 262203 64 69 7 262203 87 88 7
       262203 87 90 7 262203 87 94 7 262203 18 97 7
       262203 105 106 7 262203 105 111 7 262203 105 116 7
       262203 105 121 7 262203 105 125 7 262203 105 130 7
       262203 18 139 7 262203 105 160 7 262203 18 169 7
       262203 87 174 7 262203 64 181 7 262203 64 185 7
       262203 87 207 7 262203 105 232 7 262203 18 241 7
       262203 87 246 7 262203 87 252 7 262203 64 257 7
       262203 87 275 7 262203 64 281 7 262203 64 286 7
       262203 64 290 7 262203 87 311 7 262203 105 335 7
       262203 18 344 7 262203 18 349 7 262203 18 354 7
       262203 87 359 7 262203 64 364 7 262203 87 382 7
       262203 64 388 7 262203 64 394 7 262203 64 406 7
       262203 64 410 7 262203 87 431 7 262205 17 22 21
       327745 28 29 25 27 262205 17 30 29 327813 17 31
       22 30 196670 19 31 262205 33 36 35 262205 37 40
       39 327767 17 41 36 40 262205 17 42 19 327813 17
       43 42 41 196670 19 43 262205 17 45 19 327745 28
       47 25 46 262205 17 48 47 327813 17 49 45 48
       393281 52 53 25 46 51 262205 6 54 53 327822 17
       55 49 54 196670 44 55 327745 28 58 25 57 262205
       17 59 58 196670 56 59 327745 28 62 25 61 262205
       17 63 62 196670 60 63 327745 52 67 25 66 262205
       6 68 67 196670 65 68 196670 69 70 327745 52 72
       25 71 262205 6 73 72 327860 74 75 73 70 196855
       77 0 262394 75 76 77 131320 76 327745 64 78 19
       51 262205 6 79 78 327745 52 81 25 80 262205 6
       82 81 327864 74 83 79 82 196855 85 0 262394 83
       84 85 131320 84 65788 131320 85 131321 77 131320 77 262201
       7 89 9 196670 88 89 262205 7 92 91 393228 7
       93 1 69 92 196670 90 93 196670 94 96 393281 28
       103 102 46 46 262205 17 104 103 196670 97 104 327745
       64 108 97 107 262205 6 109 108 262254 26 110 109
       196670 106 110 327745 64 113 97 112 262205 6 114 113
       262254 26 115 114 196670 111 115 327745 64 118 97 117
       262205 6 119 118 262254 26 120 119 196670 116 120 327745
       64 122 97 51 262205 6 123 122 262254 26 124 123
       196670 121 124 196670 125 27 262205 26 126 106 327853 74
       127 126 46 196855 129 0 262394 127 128 129 131320 128
       196670 130 46 131321 131 131320 131 262390 133 134 0 131321
       135 131320 135 262205 26 136 130 262205 26 137 106 327857
       74 138 136 137 262394 138 132 133 131320 132 262205 26
       140 125 327808 26 141 140 27 196670 125 141 393281 28
       142 102 46 140 262205 17 143 142 196670 139 143 262205
       17 144 44 524367 7 145 144 144 0 1 2 262205
       17 146 139 524367 7 147 146 146 0 1 2 327813
       7 148 145 147 327745 64 149 139 51 262205 6 150
       149 327822 7 151 148 150 262205 7 152 94 327809 7
       153 152 151 196670 94 153 131321 134 131320 134 262205 26
       154 130 327808 26 155 154 27 196670 130 155 131321 131
       131320 133 131321 129 131320 129 262205 26 156 111 327853 74
       157 156 46 196855 159 0 262394 157 158 159 131320 158
       196670 160 46 131321 161 131320 161 262390 163 164 0 131321
       165 131320 165 262205 26 166 160 262205 26 167 111 327857
       74 168 166 167 262394 168 162 163 131320 162 262205 26
       170 125 327808 26 171 170 27 196670 125 171 393281 28
       172 102 46 170 262205 17 173 172 196670 169 173 262205
       26 175 125 327808 26 176 175 27 196670 125 176 393281
       28 177 102 46 175 262205 17 178 177 524367 7 179
       178 178 0 1 2 262271 7 180 179 196670 174 180
       262205 7 182 174 262205 7 183 88 327828 6 184 182
       183 196670 181 184 262205 6 186 181 458764 6 187 1
       40 186 95 196670 185 187 262205 17 188 19 524367 7
       189 188 188 0 1 2 262205 17 190 169 524367 7
       191 190 190 0 1 2 327813 7 192 189 191 262205
       6 193 185 327745 64 194 169 51 262205 6 195 194
       327813 6 196 193 195 327822 7 197 192 196 262205 7
       198 94 327809 7 199 198 197 196670 94 199 262205 6
       200 65 327866 74 201 200 95 262205 6 202 185 327866
       74 203 202 95 327847 74 204 201 203 196855 206 0
       262394 204 205 206 131320 205 262205 7 208 174 262205 7
       209 90 327809 7 210 208 209 393228 7 211 1 69
       210 196670 207 211 262205 17 212 56 524367 7 213 212
       212 0 1 2 262205 7 214 207 262205 7 215 88
       327828 6 216 214 215 458764 6 217 1 40 216 95
       262205 6 218 65 458764 6 219 1 26 217 218 327745
       64 220 169 51 262205 6 221 220 327813 6 222 219
       221 327822 7 223 213 222 262205 7 224 94 327809 7
       225 224 223 196670 94 225 131321 206 131320 206 131321 164
       131320 164 262205 26 226 160 327808 26 227 226 27 196670
       160 227 131321 161 131320 163 131321 159 131320 159 262205 26
       228 116 327853 74 229 228 46 196855 231 0 262394 229
       230 231 131320 230 196670 232 46 131321 233 131320 233 262390
       235 236 0 131321 237 131320 237 262205 26 238 232 262205
       26 239 116 327857 74 240 238 239 262394 240 234 235
       131320 234 262205 26 242 125 327808 26 243 242 27 196670
       125 243 393281 28 244 102 46 242 262205 17 245 244
       196670 241 245 262205 26 247 125 327808 26 248 247 27
       196670 125 248 393281 28 249 102 46 247 262205 17 250
       249 524367 7 251 250 250 0 1 2 196670 246 251
       262205 7 253 246 262205 7 255 254 327811 7 256 253
       255 196670 252 256 327745 64 258 252 107 262205 6 259
       258 327745 64 260 252 107 262205 6 261 260 327813 6
       262 259 261 327745 64 263 252 112 262205 6 264 263
       327745 64 265 252 112 262205 6 266 265 327813 6 267
       264 266 327809 6 268 262 267 327745 64 269 252 117
       262205 6 270 269 327745 64 271 252 117 262205 6 272
       271 327813 6 273 270 272 327809 6 274 268 273 196670
       257 274 262205 7 276 252 262205 6 277 257 393228 6
       278 1 31 277 393296 7 279 278 278 278 327816 7
       280 276 279 196670 275 280 327745 64 282 241 51 262205
       6 283 282 262205 6 284 257 327816 6 285 283 284
       196670 281 285 262205 7 287 275 262205 7 288 88 327828
       6 289 287 288 196670 286 289 262205 6 291 281 262205
       6 292 286 458764 6 293 1 40 292 95 327813 6
       294 291 293 196670 290 294 262205 17 295 19 524367 7
       296 295 295 0 1 2 262205 17 297 241 524367 7
       298 297 297 0 1 2 327813 7 299 296 298 262205
       6 300 290 327822 7 301 299 300 262205 7 302 94
       327809 7 303 302 301 196670 94 303 262205 6 304 65
       327866 74 305 304 95 262205 6 306 290 327866 74 307
       306 95 327847 74 308 305 307 196855 310 0 262394 308
       309 310 131320 309 262205 7 312 275 262205 7 313 90
       327809 7 314 312 313 393228 7 315 1 69 314 196670
       311 315 262205 17 316 56 524367 7 317 316 316 0
       1 2 262205 7 318 311 262205 7 319 88 327828 6
       320 318 319 458764 6 321 1 40 320 95 262205 6
       322 65 458764 6 323 1 26 321 322 262205 6 324
       281 327813 6 325 323 324 327822 7 326 317 325 262205
       7 327 94 327809 7 328 327 326 196670 94 328 131321
       310 131320 310 131321 236 131320 236 262205 26 329 232 327808
       26 330 329 27 196670 232 330 131321 233 131320 235 131321
       231 131320 231 262205 26 331 121 327853 74 332 331 46
       196855 334 0 262394 332 333 334 131320 333 196670 335 46
       131321 336 131320 336 262390 338 339 0 131321 340 131320 340
       262205 26 341 335 262205 26 342 121 327857 74 343 341
       342 262394 343 337 338 131320 337 262205 26 345 125 327808
       26 346 345 27 196670 125 346 393281 28 347 102 46
       345 262205 17 348 347 196670 344 348 262205 26 350 125
       327808 26 351 350 27 196670 125 351 393281 28 352 102
       46 350 262205 17 353 352 196670 349 353 262205 26 355
       125 327808 26 356 355 27 196670 125 356 393281 28 357
       102 46 355 262205 17 358 357 196670 354 358 262205 17
       360 349 524367 7 361 360 360 0 1 2 262205 7
       362 254 327811 7 363 361 362 196670 359 363 327745 64
       365 359 107 262205 6 366 365 327745 64 367 359 107
       262205 6 368 367 327813 6 369 366 368 327745 64 370
       359 112 262205 6 371 370 327745 64 372 359 112 262205
       6 373 372 327813 6 374 371 373 327809 6 375 369
       374 327745 64 376 359 117 262205 6 377 376 327745 64
       378 359 117 262205 6 379 378 327813 6 380 377 379
       327809 6 381 375 380 196670 364 381 262205 7 383 359
       262205 6 384 364 393228 6 385 1 31 384 393296 7
       386 385 385 385 327816 7 387 383 386 196670 382 387
       262205 17 389 354 524367 7 390 389 389 0 1 2
       262205 7 391 382 262271 7 392 391 327828 6 393 390
       392 196670 388 393 327745 64 395 344 51 262205 6 396
       395 327745 64 397 354 51 262205 6 398 397 327745 64
       399 349 51 262205 6 400 399 262205 6 401 388 524300
       6 402 1 49 398 400 401 327813 6 403 396 402
       262205 6 404 364 327816 6 405 403 404 196670 394 405
       262205 7 407 382 262205 7 408 88 327828 6 409 407
       408 196670 406 409 262205 6 411 394 262205 6 412 406
       458764 6 413 1 40 412 95 327813 6 414 411 413
       196670 410 414 262205 17 415 19 524367 7 416 415 415
       0 1 2 262205 17 417 344 524367 7 418 417 417
       0 1 2 327813 7 419 416 418 262205 6 420 410
       327822 7 421 419 420 262205 7 422 94 327809 7 423
       422 421 196670 94 423 262205 6 424 65 327866 74 425
       424 95 262205 6 426 410 327866 74 427 426 95 327847
       74 428 425 427 196855 430 0 262394 428 429 430 131320
       429 262205 7 432 382 262205 7 433 90 327809 7 434
       432 433 393228 7 435 1 69 434 196670 431 435 262205
       17 436 56 524367 7 437 436 436 0 1 2 262205
       7 438 431 262205 7 439 88 327828 6 440 438 439
       458764 6 441 1 40 440 95 262205 6 442 65 458764
       6 443 1 26 441 442 262205 6 444 394 327813 6
       445 443 444 327822 7 446 437 445 262205 7 447 94
       327809 7 448 447 446 196670 94 448 131321 430 131320 430
       131321 339 131320 339 262205 26 449 335 327808 26 450 449
       27 196670 335 450 131321 336 131320 338 131321 334 131320 334
       262205 7 453 94 262205 6 454 69 327822 7 455 453
       454 262205 17 456 60 524367 7 457 456 456 0 1
       2 327809 7 458 455 457 327745 459 460 452 107 327761
       6 461 458 0 196670 460 461 327745 459 462 452 112
       327761 6 463 458 1 196670 462 463 327745 459 464 452
       117 327761 6 465 458 2 196670 464 465 327745 64 466
       19 51 262205 6 467 466 327745 459 468 452 51 196670
       468 467 65789 65592 327734 7 9 0 8 131320 10 262205
       7 13 12 393228 7 14 1 69 13 131326 14 65592
    }
    NumSpecializationConstants 0
  }
}
)");
vsg::VSG io;
return io.read_cast<vsg::ShaderSet>(str);
};
