#include <vsg/io/VSG.h>
static auto assimp_phong_frag = []() {std::istringstream str(
R"(#vsga 0.2.6
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
#pragma import_defines (VSG_VIEW_LIGHT_DATA, VSG_POINT_SPRITE, VSG_DIFFUSE_MAP, VSG_GREYSACLE_DIFFUSE_MAP, VSG_EMISSIVE_MAP, VSG_LIGHTMAP_MAP, VSG_NORMAL_MAP, VSG_SPECULAR_MAP, VSG_TWOSIDED)

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

#ifdef VSG_VIEW_LIGHT_DATA
layout(set = 1, binding = 0) uniform LightData
{
    vec4 values[64];
} lightData;
#endif

layout(location = 0) in vec3 eyePos;
layout(location = 1) in vec3 normalDir;
layout(location = 2) in vec4 vertexColor;
#ifndef VSG_POINT_SPRITE
layout(location = 3) in vec2 texCoord0;
#endif
layout(location = 5) in vec3 viewDir;
layout(location = 6) in vec3 lightDir;

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

    vec4 ambientColor = vertexColor * material.ambientColor;
    vec4 diffuseColor = vertexColor * material.diffuseColor;
    vec4 specularColor = vertexColor * material.specularColor;
    vec4 emissiveColor = vertexColor * material.emissiveColor;
    float shininess = material.shininess;
    float ambientOcclusion = 1.0;

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

#ifdef VSG_EMISSIVE_MAP
    emissiveColor *= texture(emissiveMap, texCoord0.st);
#endif

#ifdef VSG_LIGHTMAP_MAP
    ambientOcclusion *= texture(aoMap, texCoord0.st).r;
#endif

#ifdef VSG_SPECULAR_MAP
    specularColor *= texture(specularMap, texCoord0.st);
#endif


#if defined(VSG_VIEW_LIGHT_DATA)
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec3 nd = getNormal();
    vec3 vd = normalize(viewDir);

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
            color.rgb += ambientColor.rgb * lightColor.rgb * ambientColor.a;
        }
    }

    if (numDirectionalLights>0)
    {
        // directional lights
        for(int i = 0; i<numDirectionalLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            vec3 direction = -lightData.values[index++].xyz;
            float diff = max(dot(direction, nd), 0.0);
            color.rgb += (diffuseColor.rgb * lightColor.rgb) * (diff * diffuseColor.a);
            if (diff > 0.0)
            {
                vec3 halfDir = normalize(direction + vd);
                color.rgb += specularColor.rgb * (pow(max(dot(halfDir, nd), 0.0), shininess) * diffuseColor.a);
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
            vec3 delta = eyePos - position;
            float distance2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            vec3 direction = delta / sqrt(distance2);
            float scale = lightColor.a / distance2;

            float diff = scale * max(-dot(direction, nd), 0.0);
            color.rgb += (diffuseColor.rgb * lightColor.rgb) * diff;
            if (diff > 0.0)
            {
                vec3 halfDir = normalize(-direction + vd);
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

            vec3 delta = eyePos - position_cosInnerAngle.xyz;
            float distance2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            vec3 direction = delta / sqrt(distance2);

            float dot_lightdirection = dot(lightDirection_cosOuterAngle.xyz, direction);
            float scale = (lightColor.a  * smoothstep(lightDirection_cosOuterAngle.w, position_cosInnerAngle.w, dot_lightdirection)) / distance2;

            float diff = scale * max(-dot(direction, nd), 0.0);
            color.rgb += (diffuseColor.rgb * lightColor.rgb) * diff;
            if (diff > 0.0)
            {
                vec3 halfDir = normalize(-direction + vd);
                color.rgb += specularColor.rgb * (pow(max(dot(halfDir, nd), 0.0), shininess) * scale);
            }
        }
    }

    outColor.rgb = (color * ambientOcclusion) + emissiveColor.rgb;
#else
    vec3 nd = getNormal();
    vec3 ld = normalize(lightDir);
    vec3 vd = normalize(viewDir);

    vec3 colorFrontFace = computeLighting(ambientColor.rgb, diffuseColor.rgb, specularColor.rgb, emissiveColor.rgb, shininess, ambientOcclusion, ld, nd, vd);
#ifdef VSG_TWOSIDED
    vec3 colorBackFace = computeLighting(ambientColor.rgb, diffuseColor.rgb, specularColor.rgb, emissiveColor.rgb, shininess, ambientOcclusion, ld, -nd, vd);
    outColor.rgb = colorFrontFace + colorBackFace;
#else
    outColor.rgb = colorFrontFace;
#endif
#endif

    outColor.a = diffuseColor.a;
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
