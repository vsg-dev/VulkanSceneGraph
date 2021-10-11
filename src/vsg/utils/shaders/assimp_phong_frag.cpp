#include <vsg/io/VSG.h>
static auto assimp_phong_frag = []() {std::istringstream str(
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
#pragma import_defines (VSG_POINT_SPRITE, VSG_DIFFUSE_MAP, VSG_GREYSACLE_DIFFUSE_MAP, VSG_EMISSIVE_MAP, VSG_LIGHTMAP_MAP, VSG_NORMAL_MAP, VSG_SPECULAR_MAP, VSG_TWOSIDED)

#ifdef VSG_DIFFUSE_MAP
layout(binding = 0) uniform sampler2D diffuseMap;
#endif

#ifdef VSG_NORMAL_MAP
layout(binding = 2) uniform sampler2D normalMap;
#endif

#ifdef VSG_LIGHTMAP_MAP
layout(binding = 3) uniform sampler2D aoMap;
#endif

#ifdef VSG_EMISSIVE_MAP
layout(binding = 4) uniform sampler2D emissiveMap;
#endif

#ifdef VSG_SPECULAR_MAP
layout(binding = 5) uniform sampler2D specularMap;
#endif

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;

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
    outColor.a = diffuseColor.a;
}
"
    hints id=0
    SPIRVSize 1323
    SPIRV 119734787 65536 524298 185 0 131089 1 393227 1 1280527431 1685353262 808793134
     0 196622 0 1 786447 4 4 1852399981 0 26 81 137
     141 169 181 184 196624 4 7 196611 2 450 589828 1096764487
     1935622738 1918988389 1600484449 1684105331 1868526181 1667590754 29556 262149 4 1852399981 0 327685
     9 1316250983 1634562671 10348 983045 23 1886220131 1281717365 1952999273 677867113 993224310 993224310
     993224310 993224310 1715155302 1719024433 1719024435 1719024435 15155 393221 14 1768058209 1131703909 1919904879
     0 393221 15 1717987684 1130722165 1919904879 0 393221 16 1667592307 1918987381 1869377347
     114 393221 17 1936289125 1702259059 1869377347 114 327685 18 1852401779 1936027241 115
     458757 19 1768058209 1333030501 1970037603 1852795251 0 196613 20 25708 196613 21
     25710 196613 22 25718 327685 26 1836216174 1766091873 114 262149 31 1869377379
     114 262149 37 1717987684 0 262149 52 1718378856 7498052 262149 67 1970496882
     29804 393221 79 1768058209 1131703909 1919904879 0 327685 81 1953654134 1866692709 7499628
     393221 83 1702125901 1818323314 1635017028 0 458758 83 0 1768058209 1131703909 1919904879
     0 458758 83 1 1717987684 1130722165 1919904879 0 458758 83 2 1667592307
     1918987381 1869377347 114 458758 83 3 1936289125 1702259059 1869377347 114 393222 83
     4 1852401779 1936027241 115 393222 83 5 1752198241 1935756641 107 458758 83
     6 1752198241 1935756641 1953842027 6710895 327685 85 1702125933 1818323314 0 393221 92
     1717987684 1130722165 1919904879 0 393221 98 1667592307 1918987381 1869377347 114 393221 104
     1936289125 1702259059 1869377347 114 327685 110 1852401779 1936027241 115 458757 115 1768058209
     1333030501 1970037603 1852795251 0 196613 134 25710 196613 136 25708 327685 137
     1751607660 1919501428 0 196613 140 25718 262149 141 2003134838 7498052 393221 144
     1869377379 1869760114 1632007278 25955 262149 145 1634886000 109 262149 148 1634886000 109
     262149 151 1634886000 109 262149 154 1634886000 109 262149 157 1634886000 109
     262149 159 1634886000 109 262149 161 1634886000 109 262149 163 1634886000 109
     262149 165 1634886000 109 327685 169 1131705711 1919904879 0 393221 178 1752397136
     1936617283 1953390964 115 393222 178 0 1785688688 1769235301 28271 393222 178 1
     1701080941 1701402220 119 196613 180 25456 262149 181 1348827493 29551 327685 184
     1131963764 1685221231 48 262215 26 30 1 262215 81 30 2 327752
     83 0 35 0 327752 83 1 35 16 327752 83 2
     35 32 327752 83 3 35 48 327752 83 4 35 64
     327752 83 5 35 68 327752 83 6 35 72 196679 83
     2 262215 85 34 0 262215 85 33 10 262215 137 30
     6 262215 141 30 5 262215 169 30 0 262216 178 0
     5 327752 178 0 35 0 327752 178 0 7 16 262216
     178 1 5 327752 178 1 35 64 327752 178 1 7
     16 196679 178 2 262215 181 30 0 262215 184 30 3
     131091 2 196641 3 2 196630 6 32 262167 7 6 3
     196641 8 7 262176 11 7 7 262176 12 7 6 786465
     13 7 11 11 11 11 12 12 11 11 11 262176
     25 1 7 262203 25 26 1 262187 6 32 0 393260
     7 33 32 32 32 131092 48 262167 77 6 4 262176
     78 7 77 262176 80 1 77 262203 80 81 1 589854
     83 77 77 77 77 6 6 6 262176 84 2 83
     262203 84 85 2 262165 86 32 1 262187 86 87 0
     262176 88 2 77 262187 86 94 1 262187 86 100 2
     262187 86 106 3 262187 86 111 4 262176 112 2 6
     262187 6 116 1065353216 262187 86 117 5 262165 123 32 0
     262187 123 124 3 262187 86 127 6 262203 25 137 1
     262203 25 141 1 262176 168 3 77 262203 168 169 3
     262176 175 3 6 262168 177 77 4 262174 178 177 177
     262176 179 9 178 262203 179 180 9 262203 25 181 1
     262167 182 6 2 262176 183 1 182 262203 183 184 1
     327734 2 4 0 3 131320 5 262203 78 79 7 262203
     78 92 7 262203 78 98 7 262203 78 104 7 262203
     12 110 7 262203 12 115 7 262203 11 134 7 262203
     11 136 7 262203 11 140 7 262203 11 144 7 262203
     11 145 7 262203 11 148 7 262203 11 151 7 262203
     11 154 7 262203 12 157 7 262203 12 159 7 262203
     11 161 7 262203 11 163 7 262203 11 165 7 262205
     77 82 81 327745 88 89 85 87 262205 77 90 89
     327813 77 91 82 90 196670 79 91 262205 77 93 81
     327745 88 95 85 94 262205 77 96 95 327813 77 97
     93 96 196670 92 97 262205 77 99 81 327745 88 101
     85 100 262205 77 102 101 327813 77 103 99 102 196670
     98 103 262205 77 105 81 327745 88 107 85 106 262205
     77 108 107 327813 77 109 105 108 196670 104 109 327745
     112 113 85 111 262205 6 114 113 196670 110 114 196670
     115 116 327745 112 118 85 117 262205 6 119 118 327860
     48 120 119 116 196855 122 0 262394 120 121 122 131320
     121 327745 12 125 92 124 262205 6 126 125 327745 112
     128 85 127 262205 6 129 128 327864 48 130 126 129
     196855 132 0 262394 130 131 132 131320 131 65788 131320 132
     131321 122 131320 122 262201 7 135 9 196670 134 135 262205
     7 138 137 393228 7 139 1 69 138 196670 136 139
     262205 7 142 141 393228 7 143 1 69 142 196670 140
     143 262205 77 146 79 524367 7 147 146 146 0 1
     2 196670 145 147 262205 77 149 92 524367 7 150 149
     149 0 1 2 196670 148 150 262205 77 152 98 524367
     7 153 152 152 0 1 2 196670 151 153 262205 77
     155 104 524367 7 156 155 155 0 1 2 196670 154
     156 262205 6 158 110 196670 157 158 262205 6 160 115
     196670 159 160 262205 7 162 136 196670 161 162 262205 7
     164 134 196670 163 164 262205 7 166 140 196670 165 166
     852025 7 167 23 145 148 151 154 157 159 161 163
     165 196670 144 167 262205 7 170 144 262205 77 171 169
     589903 77 172 171 170 4 5 6 3 196670 169 172
     327745 12 173 92 124 262205 6 174 173 327745 175 176
     169 124 196670 176 174 65789 65592 327734 7 9 0 8
     131320 10 262205 7 27 26 393228 7 28 1 69 27
     131326 28 65592 327734 7 23 0 13 196663 11 14 196663
     11 15 196663 11 16 196663 11 17 196663 12 18 196663
     12 19 196663 11 20 196663 11 21 196663 11 22 131320
     24 262203 11 31 7 262203 12 37 7 262203 11 52
     7 262203 11 67 7 196670 31 33 262205 7 34 14
     262205 7 35 31 327809 7 36 35 34 196670 31 36
     262205 7 38 20 262205 7 39 21 327828 6 40 38
     39 458764 6 41 1 40 40 32 196670 37 41 262205
     7 42 15 262205 6 43 37 327822 7 44 42 43
     262205 7 45 31 327809 7 46 45 44 196670 31 46
     262205 6 47 37 327866 48 49 47 32 196855 51 0
     262394 49 50 51 131320 50 262205 7 53 20 262205 7
     54 22 327809 7 55 53 54 393228 7 56 1 69
     55 196670 52 56 262205 7 57 16 262205 7 58 52
     262205 7 59 21 327828 6 60 58 59 458764 6 61
     1 40 60 32 262205 6 62 18 458764 6 63 1
     26 61 62 327822 7 64 57 63 262205 7 65 31
     327809 7 66 65 64 196670 31 66 131321 51 131320 51
     262205 7 68 31 262205 7 69 17 327809 7 70 68
     69 196670 67 70 262205 6 71 19 262205 7 72 67
     327822 7 73 72 71 196670 67 73 262205 7 74 67
     131326 74 65592
  }
  NumSpecializationConstants 0
}
)");
vsg::VSG io;
return io.read_cast<vsg::ShaderStage>(str);
};
