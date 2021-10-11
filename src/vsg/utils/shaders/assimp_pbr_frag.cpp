#include <vsg/io/VSG.h>
static auto assimp_pbr_frag = []() {std::istringstream str(
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
#pragma import_defines (VSG_DIFFUSE_MAP, VSG_GREYSACLE_DIFFUSE_MAP, VSG_EMISSIVE_MAP, VSG_LIGHTMAP_MAP, VSG_NORMAL_MAP, VSG_METALLROUGHNESS_MAP, VSG_SPECULAR_MAP, VSG_TWOSIDED, VSG_WORKFLOW_SPECGLOSS)

const float PI = 3.14159265359;
const float RECIPROCAL_PI = 0.31830988618;
const float RECIPROCAL_PI2 = 0.15915494;
const float EPSILON = 1e-6;
const float c_MinRoughness = 0.04;

#ifdef VSG_DIFFUSE_MAP
layout(binding = 0) uniform sampler2D diffuseMap;
#endif

#ifdef VSG_METALLROUGHNESS_MAP
layout(binding = 1) uniform sampler2D mrMap;
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

layout(binding = 10) uniform PbrData
{
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 diffuseFactor;
    vec4 specularFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaMask;
    float alphaMaskCutoff;
} pbr;

layout(location = 0) in vec3 eyePos;
layout(location = 1) in vec3 normalDir;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec2 texCoord0;
layout(location = 5) in vec3 viewDir;
layout(location = 6) in vec3 lightDir;

layout(location = 0) out vec4 outColor;


// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo
{
    float NdotL;                  // cos angle between normal and light direction
    float NdotV;                  // cos angle between normal and view direction
    float NdotH;                  // cos angle between normal and half vector
    float LdotH;                  // cos angle between light direction and half vector
    float VdotH;                  // cos angle between view direction and half vector
    float VdotL;                  // cos angle between view direction and light direction
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor;            // color contribution from diffuse lighting
    vec3 specularColor;           // color contribution from specular lighting
};


vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    vec3 linOut = pow(srgbIn.xyz, vec3(2.2));
    return vec4(linOut,srgbIn.w);
}

vec4 LINEARtoSRGB(vec4 srgbIn)
{
    vec3 linOut = pow(srgbIn.xyz, vec3(1.0 / 2.2));
    return vec4(linOut, srgbIn.w);
}

float rcp(const in float value)
{
    return 1.0 / value;
}

float pow5(const in float value)
{
    return value * value * value * value * value;
}

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

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 BRDF_Diffuse_Lambert(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor * RECIPROCAL_PI;
}

vec3 BRDF_Diffuse_Custom_Lambert(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor * RECIPROCAL_PI * pow(pbrInputs.NdotV, 0.5 + 0.3 * pbrInputs.perceptualRoughness);
}

// [Gotanda 2012, \"Beyond a Simple Physically Based Blinn-Phong Model in Real-Time\"]
vec3 BRDF_Diffuse_OrenNayar(PBRInfo pbrInputs)
{
    float a = pbrInputs.alphaRoughness;
    float s = a;// / ( 1.29 + 0.5 * a );
    float s2 = s * s;
    float VoL = 2 * pbrInputs.VdotH * pbrInputs.VdotH - 1;		// double angle identity
    float Cosri = pbrInputs.VdotL - pbrInputs.NdotV * pbrInputs.NdotL;
    float C1 = 1 - 0.5 * s2 / (s2 + 0.33);
    float C2 = 0.45 * s2 / (s2 + 0.09) * Cosri * ( Cosri >= 0 ? 1.0 / max(pbrInputs.NdotL, pbrInputs.NdotV) : 1 );
    return pbrInputs.diffuseColor / PI * ( C1 + C2 ) * ( 1 + pbrInputs.perceptualRoughness * 0.5 );
}

// [Gotanda 2014, \"Designing Reflectance Models for New Consoles\"]
vec3 BRDF_Diffuse_Gotanda(PBRInfo pbrInputs)
{
    float a = pbrInputs.alphaRoughness;
    float a2 = a * a;
    float F0 = 0.04;
    float VoL = 2 * pbrInputs.VdotH * pbrInputs.VdotH - 1;		// double angle identity
    float Cosri = VoL - pbrInputs.NdotV * pbrInputs.NdotL;
    float a2_13 = a2 + 1.36053;
    float Fr = ( 1 - ( 0.542026*a2 + 0.303573*a ) / a2_13 ) * ( 1 - pow( 1 - pbrInputs.NdotV, 5 - 4*a2 ) / a2_13 ) * ( ( -0.733996*a2*a + 1.50912*a2 - 1.16402*a ) * pow( 1 - pbrInputs.NdotV, 1 + rcp(39*a2*a2+1) ) + 1 );
    //float Fr = ( 1 - 0.36 * a ) * ( 1 - pow( 1 - NoV, 5 - 4*a2 ) / a2_13 ) * ( -2.5 * Roughness * ( 1 - NoV ) + 1 );
    float Lm = ( max( 1 - 2*a, 0 ) * ( 1 - pow5( 1 - pbrInputs.NdotL ) ) + min( 2*a, 1 ) ) * ( 1 - 0.5*a * (pbrInputs.NdotL - 1) ) * pbrInputs.NdotL;
    float Vd = ( a2 / ( (a2 + 0.09) * (1.31072 + 0.995584 * pbrInputs.NdotV) ) ) * ( 1 - pow( 1 - pbrInputs.NdotL, ( 1 - 0.3726732 * pbrInputs.NdotV * pbrInputs.NdotV ) / ( 0.188566 + 0.38841 * pbrInputs.NdotV ) ) );
    float Bp = Cosri < 0 ? 1.4 * pbrInputs.NdotV * pbrInputs.NdotL * Cosri : Cosri;
    float Lr = (21.0 / 20.0) * (1 - F0) * ( Fr * Lm + Vd + Bp );
    return pbrInputs.diffuseColor * RECIPROCAL_PI * Lr;
}

vec3 BRDF_Diffuse_Burley(PBRInfo pbrInputs)
{
    float energyBias = mix(pbrInputs.perceptualRoughness, 0.0, 0.5);
    float energyFactor = mix(pbrInputs.perceptualRoughness, 1.0, 1.0 / 1.51);
    float fd90 = energyBias + 2.0 * pbrInputs.VdotH * pbrInputs.VdotH * pbrInputs.perceptualRoughness;
    float f0 = 1.0;
    float lightScatter = f0 + (fd90 - f0) * pow(1.0 - pbrInputs.NdotL, 5.0);
    float viewScatter = f0 + (fd90 - f0) * pow(1.0 - pbrInputs.NdotV, 5.0);

    return pbrInputs.diffuseColor * lightScatter * viewScatter * energyFactor;
}

vec3 BRDF_Diffuse_Disney(PBRInfo pbrInputs)
{
	float Fd90 = 0.5 + 2.0 * pbrInputs.perceptualRoughness * pbrInputs.VdotH * pbrInputs.VdotH;
    vec3 f0 = vec3(0.1);
	vec3 invF0 = vec3(1.0, 1.0, 1.0) - f0;
	float dim = min(invF0.r, min(invF0.g, invF0.b));
	float result = ((1.0 + (Fd90 - 1.0) * pow(1.0 - pbrInputs.NdotL, 5.0 )) * (1.0 + (Fd90 - 1.0) * pow(1.0 - pbrInputs.NdotV, 5.0 ))) * dim;
	return pbrInputs.diffuseColor * result;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)
{
    //return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance90*pbrInputs.reflectance0) * exp2((-5.55473 * pbrInputs.VdotH - 6.98316) * pbrInputs.VdotH);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r + (1.0 - r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r + (1.0 - r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from \"Average Irregularity Representation of a Roughened Surface for Ray Reflection\" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (PI * f * f);
}

vec3 BRDF(vec3 v, vec3 n, vec3 l, vec3 h, float perceptualRoughness, float metallic, vec3 specularEnvironmentR0, vec3 specularEnvironmentR90, float alphaRoughness, vec3 diffuseColor, vec3 specularColor, float ao)
{
    vec3 reflection = -normalize(reflect(v, n));
    reflection.y *= -1.0f;

    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);
    float VdotL = clamp(dot(v, l), 0.0, 1.0);

    PBRInfo pbrInputs = PBRInfo(NdotL,
                                NdotV,
                                NdotH,
                                LdotH,
                                VdotH,
                                VdotL,
                                perceptualRoughness,
                                metallic,
                                specularEnvironmentR0,
                                specularEnvironmentR90,
                                alphaRoughness,
                                diffuseColor,
                                specularColor);

    // Calculate the shading terms for the microfacet specular shading model
    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    const vec3 u_LightColor = vec3(1.0);

    // Calculation of analytical lighting contribution
    vec3 diffuseContrib = (1.0 - F) * BRDF_Diffuse_Disney(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
    vec3 color = NdotL * u_LightColor * (diffuseContrib + specContrib);

    color *= ao;

#ifdef VSG_EMISSIVE_MAP
    vec3 emissive = SRGBtoLINEAR(texture(emissiveMap, texCoord0)).rgb * pbr.emissiveFactor.rgb;
#else
    vec3 emissive = pbr.emissiveFactor.rgb;
#endif
    color += emissive;

    return color;
}

float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular)
{
    float perceivedDiffuse = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g + 0.114 * diffuse.b * diffuse.b);
    float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g + 0.114 * specular.b * specular.b);

    if (perceivedSpecular < c_MinRoughness)
    {
        return 0.0;
    }

    float a = c_MinRoughness;
    float b = perceivedDiffuse * (1.0 - maxSpecular) / (1.0 - c_MinRoughness) + perceivedSpecular - 2.0 * c_MinRoughness;
    float c = c_MinRoughness - perceivedSpecular;
    float D = max(b * b - 4.0 * a * c, 0.0);
    return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

void main()
{
    float perceptualRoughness = 0.0;
    float metallic;
    vec3 diffuseColor;
    vec4 baseColor;

    float ambientOcclusion = 1.0;

    vec3 f0 = vec3(0.04);

#ifdef VSG_DIFFUSE_MAP
    #ifdef VSG_GREYSACLE_DIFFUSE_MAP
        float v = texture(diffuseMap, texCoord0.st).s * pbr.baseColorFactor;
        baseColor = vertexColor * vec4(v, v, v, 1.0);
    #else
        baseColor = vertexColor * SRGBtoLINEAR(texture(diffuseMap, texCoord0)) * pbr.baseColorFactor;
    #endif
#else
    baseColor = vertexColor * pbr.baseColorFactor;
#endif

    if (pbr.alphaMask == 1.0f)
    {
        if (baseColor.a < pbr.alphaMaskCutoff)
            discard;
    }


#ifdef VSG_WORKFLOW_SPECGLOSS
    #ifdef VSG_SPECULAR_MAP
        vec3 specular = SRGBtoLINEAR(texture(specularMap, texCoord0)).rgb;
        perceptualRoughness = 1.0 - texture(specularMap, texCoord0).a;
    #else
        vec3 specular = vec3(0.0);
        perceptualRoughness = 0.0;
    #endif

        const float epsilon = 1e-6;

    #ifdef VSG_DIFFUSE_MAP
        vec4 diffuse = SRGBtoLINEAR(texture(diffuseMap, texCoord0));
    #else
        vec4 diffuse = vec4(1.0);
    #endif

        float maxSpecular = max(max(specular.r, specular.g), specular.b);

        // Convert metallic value from specular glossiness inputs
        metallic = convertMetallic(diffuse.rgb, specular, maxSpecular);

        vec3 baseColorDiffusePart = diffuse.rgb * ((1.0 - maxSpecular) / (1 - c_MinRoughness) / max(1 - metallic, epsilon)) * pbr.diffuseFactor.rgb;
        vec3 baseColorSpecularPart = specular - (vec3(c_MinRoughness) * (1 - metallic) * (1 / max(metallic, epsilon))) * pbr.specularFactor.rgb;
        baseColor = vec4(mix(baseColorDiffusePart, baseColorSpecularPart, metallic * metallic), diffuse.a);
#else
        perceptualRoughness = pbr.roughnessFactor;
        metallic = pbr.metallicFactor;

    #ifdef VSG_METALLROUGHNESS_MAP
        vec4 mrSample = texture(mrMap, texCoord0);
        perceptualRoughness = mrSample.g * perceptualRoughness;
        metallic = mrSample.b * metallic;
    #endif
#endif

#ifdef VSG_LIGHTMAP_MAP
    ambientOcclusion = texture(aoMap, texCoord0).r;
#endif

    diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
    diffuseColor *= 1.0 - metallic;

    float alphaRoughness = perceptualRoughness * perceptualRoughness;

    vec3 specularColor = mix(f0, baseColor.rgb, metallic);

    // Compute reflectance.
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
    // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = specularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

    vec3 n = getNormal();
    vec3 v = normalize(viewDir);    // Vector from surface point to camera
    vec3 l = normalize(lightDir);     // Vector from surface point to light
    vec3 h = normalize(l+v);                        // Half vector between both l and v

    vec3 colorFrontFace = BRDF(v, n, l, h, perceptualRoughness, metallic, specularEnvironmentR0, specularEnvironmentR90, alphaRoughness, diffuseColor, specularColor, ambientOcclusion);
#ifdef VSG_TWOSIDED
    vec3 colorBackFace = BRDF(v, -n, l, h, perceptualRoughness, metallic, specularEnvironmentR0, specularEnvironmentR90, alphaRoughness, diffuseColor, specularColor, ambientOcclusion);
    vec3 color = colorFrontFace+colorBackFace;
#else
    vec3 color = colorFrontFace;
#endif

    outColor = LINEARtoSRGB(vec4(color, baseColor.a));
}
"
    hints id=0
    SPIRVSize 3246
    SPIRV 119734787 65536 524298 496 0 131089 1 393227 1 1280527431 1685353262 808793134
     0 196622 0 1 786447 4 4 1852399981 0 68 364 434
     438 475 492 495 196624 4 7 196611 2 450 589828 1096764487
     1935622738 1918988389 1600484449 1684105331 1868526181 1667590754 29556 262149 4 1852399981 0 458757
     11 1162758476 1869894209 1111970387 879130152 59 262149 10 1650946675 28233 327685 15
     1316250983 1634562671 10348 262149 17 1230127696 7300718 327686 17 0 1953457230 76
     327686 17 1 1953457230 86 327686 17 2 1953457230 72 327686 17
     3 1953457228 72 327686 17 4 1953457238 72 327686 17 5 1953457238
     76 524294 17 6 1668441456 1970565221 1867672673 1852335989 7566181 393222 17 7
     1635018093 1936027244 115 458758 17 8 1818649970 1635017573 811950958 0 458758 17
     9 1818649970 1635017573 962945902 48 458758 17 10 1752198241 1970229857 1701734503 29555
     458758 17 11 1717987684 1130722165 1919904879 0 458758 17 12 1667592307 1918987381
     1869377347 114 1441797 21 1178882626 1718174815 1702065510 1936278623 679044462 1970435187 1345156195 1850298946
     1714253670 828779825 758212141 1714237798 828779825 758212141 1982673254 1982673766 1714238310 1719020849 1719020851 3879219
     327685 20 1232233072 1953853550 115 1441797 24 1667592307 1918987381 1818649938 1769235301 1932029551
     1668641396 1112550772 1718503762 828779887 758212141 1714237798 828779825 758212141 1714237798 1719020849 1719020851 828779827
     862352941 862352941 15153 327685 23 1232233072 1953853550 115 1441797 28 1836016999 1769108581
     1667452771 1769174380 1932029551 1668641396 1112550772 1718503762 828779887 758212141 1714237798 828779825 758212141 1714237798
     1719020849 1719020851 828779827 862352941 862352941 15153 327685 27 1232233072 1953853550 115 1507333
     31 1919117677 1667327599 1766093925 1769108595 1769239906 1932029551 1668641396 1112550772 1718503762 828779887 758212141
     1714237798 828779825 758212141 1714237798 1719020849 1719020851 828779827 862352941 862352941 15153 327685 30
     1232233072 1953853550 115 983045 48 1178882626 862352936 862352955 862352955 862352955 993093179 1983590758
     1983591270 1715155814 1719024433 1719024435 828783411 59 196613 36 118 196613 37 110
     196613 38 108 196613 39 104 458757 40 1668441456 1970565221 1867672673 1852335989
     7566181 327685 41 1635018093 1667853420 0 524293 42 1667592307 1918987381 1769369157 1835954034
     1383362149 48 524293 43 1667592307 1918987381 1769369157 1835954034 1383362149 12345 393221 44
     1752198241 1970229857 1701734503 29555 393221 45 1717987684 1130722165 1919904879 0 393221 46
     1667592307 1918987381 1869377347 114 196613 47 28513 262149 50 1332636012 29813 327685
     68 1836216174 1766091873 114 262149 73 809067590 0 196613 89 12390 262149
     92 1182166633 48 196613 97 7170404 262149 109 1970496882 29804 262149 165
     1953457230 76 262149 168 1953457230 86 196613 171 114 393221 178 1702130785
     1952544110 1282305897 0 393221 193 1702130785 1952544110 1450078057 0 327685 213 1735749490
     1936027240 7426931 196613 219 102 327685 241 1818649970 1769235301 28271 262149 252
     1953457230 76 262149 258 1953457230 86 262149 264 1953457230 72 262149 270
     1953457228 72 262149 275 1953457238 72 262149 280 1953457238 76 327685 285
     1232233072 1953853550 115 196613 300 70 262149 301 1634886000 109 196613 304
     71 262149 305 1634886000 109 196613 308 68 262149 309 1634886000 109
     393221 312 1717987684 1130722165 1920233071 25193 262149 316 1634886000 109 327685 320
     1667592307 1953394499 6449522 262149 333 1869377379 114 327685 343 1936289125 1702259059 0
     262149 344 1148346960 6386785 458758 344 0 1702060386 1869377347 1667319410 7499636 458758
     344 1 1936289125 1702259059 1952670022 29295 458758 344 2 1717987684 1181053813 1869898593
     114 458758 344 3 1667592307 1918987381 1952670022 29295 458758 344 4 1635018093
     1667853420 1952670022 29295 458758 344 5 1735749490 1936027240 1667319411 7499636 393222 344
     6 1752198241 1935756641 107 458758 344 7 1752198241 1935756641 1953842027 6710895 196613
     346 7496304 458757 357 1668441456 1970565221 1867672673 1852335989 7566181 458757 358 1768058209
     1333030501 1970037603 1852795251 0 196613 359 12390 327685 362 1702060386 1869377347 114
     327685 364 1953654134 1866692709 7499628 327685 388 1635018093 1667853420 0 393221 391
     1717987684 1130722165 1919904879 0 393221 401 1752198241 1970229857 1701734503 29555 393221 405
     1667592307 1918987381 1869377347 114 327685 412 1818649970 1635017573 6644590 393221 421 1818649970
     1635017573 962945902 48 524293 426 1667592307 1918987381 1769369157 1835954034 1383362149 48 524293
     428 1667592307 1918987381 1769369157 1835954034 1383362149 12345 196613 431 110 196613 433
     118 262149 434 2003134838 7498052 196613 437 108 327685 438 1751607660 1919501428
     0 196613 441 104 393221 446 1869377379 1869760114 1632007278 25955 262149 447
     1634886000 109 262149 449 1634886000 109 262149 451 1634886000 109 262149 453
     1634886000 109 262149 455 1634886000 109 262149 457 1634886000 109 262149 459
     1634886000 109 262149 461 1634886000 109 262149 463 1634886000 109 262149 465
     1634886000 109 262149 467 1634886000 109 262149 469 1634886000 109 262149 472
     1869377379 114 327685 475 1131705711 1919904879 0 262149 483 1634886000 109 393221
     489 1752397136 1936617283 1953390964 115 393222 489 0 1785688688 1769235301 28271 393222
     489 1 1701080941 1701402220 119 196613 491 25456 262149 492 1348827493 29551
     327685 495 1131963764 1685221231 48 262215 68 30 1 327752 344 0
     35 0 327752 344 1 35 16 327752 344 2 35 32
     327752 344 3 35 48 327752 344 4 35 64 327752 344
     5 35 68 327752 344 6 35 72 327752 344 7 35
     76 196679 344 2 262215 346 34 0 262215 346 33 10
     262215 364 30 2 262215 434 30 5 262215 438 30 6
     262215 475 30 0 262216 489 0 5 327752 489 0 35
     0 327752 489 0 7 16 262216 489 1 5 327752 489
     1 35 64 327752 489 1 7 16 196679 489 2 262215
     492 30 0 262215 495 30 3 131091 2 196641 3 2
     196630 6 32 262167 7 6 4 262176 8 7 7 262177
     9 7 8 262167 13 6 3 196641 14 13 983070 17
     6 6 6 6 6 6 6 6 13 13 6 13
     13 262176 18 7 17 262177 19 13 18 262177 26 6
     18 262176 33 7 13 262176 34 7 6 983073 35 13
     33 33 33 33 34 34 33 33 34 33 33 34
     262187 6 53 1055439407 393260 13 54 53 53 53 262165 57
     32 0 262187 57 58 3 262176 67 1 13 262203 67
     68 1 262187 6 74 1056964608 262187 6 75 1073741824 262165 76
     32 1 262187 76 77 6 262187 76 81 4 262187 6
     90 1036831949 393260 13 91 90 90 90 262187 6 93 1065353216
     393260 13 94 93 93 93 262187 57 98 0 262187 57
     101 1 262187 57 104 2 262187 76 112 0 262187 6
     116 1084227584 262187 76 122 1 262187 76 132 11 262187 76
     139 8 262187 76 142 9 262187 6 151 3232874585 262187 6
     155 1088386572 262187 76 172 10 262187 76 220 2 262187 6
     233 1078530011 262187 6 247 3212836864 262187 6 256 981668463 262187 6
     268 0 262187 6 326 1082130432 655390 344 7 7 7 7
     6 6 6 6 262176 345 2 344 262203 345 346 2
     262176 347 2 7 262187 6 360 1025758986 393260 13 361 360
     360 360 262176 363 1 7 262203 363 364 1 262176 369
     2 6 131092 372 262187 76 378 7 262187 76 385 5
     262187 6 423 1103626240 262203 67 434 1 262203 67 438 1
     262176 474 3 7 262203 474 475 3 262187 6 485 1050868099
     262187 6 486 1042479491 262187 6 487 897988541 262168 488 7 4
     262174 489 488 488 262176 490 9 489 262203 490 491 9
     262203 67 492 1 262167 493 6 2 262176 494 1 493
     262203 494 495 1 327734 2 4 0 3 131320 5 262203
     34 357 7 262203 34 358 7 262203 33 359 7 262203
     8 362 7 262203 34 388 7 262203 33 391 7 262203
     34 401 7 262203 33 405 7 262203 34 412 7 262203
     34 421 7 262203 33 426 7 262203 33 428 7 262203
     33 431 7 262203 33 433 7 262203 33 437 7 262203
     33 441 7 262203 33 446 7 262203 33 447 7 262203
     33 449 7 262203 33 451 7 262203 33 453 7 262203
     34 455 7 262203 34 457 7 262203 33 459 7 262203
     33 461 7 262203 34 463 7 262203 33 465 7 262203
     33 467 7 262203 34 469 7 262203 33 472 7 262203
     8 483 7 196670 357 268 196670 358 93 196670 359 361
     262205 7 365 364 327745 347 366 346 112 262205 7 367
     366 327813 7 368 365 367 196670 362 368 327745 369 370
     346 77 262205 6 371 370 327860 372 373 371 93 196855
     375 0 262394 373 374 375 131320 374 327745 34 376 362
     58 262205 6 377 376 327745 369 379 346 378 262205 6
     380 379 327864 372 381 377 380 196855 383 0 262394 381
     382 383 131320 382 65788 131320 383 131321 375 131320 375 327745
     369 386 346 385 262205 6 387 386 196670 357 387 327745
     369 389 346 81 262205 6 390 389 196670 388 390 262205
     7 392 362 524367 13 393 392 392 0 1 2 262205
     13 394 359 327811 13 395 94 394 327813 13 396 393
     395 196670 391 396 262205 6 397 388 327811 6 398 93
     397 262205 13 399 391 327822 13 400 399 398 196670 391
     400 262205 6 402 357 262205 6 403 357 327813 6 404
     402 403 196670 401 404 262205 13 406 359 262205 7 407
     362 524367 13 408 407 407 0 1 2 262205 6 409
     388 393296 13 410 409 409 409 524300 13 411 1 46
     406 408 410 196670 405 411 327745 34 413 405 98 262205
     6 414 413 327745 34 415 405 101 262205 6 416 415
     458764 6 417 1 40 414 416 327745 34 418 405 104
     262205 6 419 418 458764 6 420 1 40 417 419 196670
     412 420 262205 6 422 412 327813 6 424 422 423 524300
     6 425 1 43 424 268 93 196670 421 425 262205 13
     427 405 196670 426 427 262205 6 429 421 327822 13 430
     94 429 196670 428 430 262201 13 432 15 196670 431 432
     262205 13 435 434 393228 13 436 1 69 435 196670 433
     436 262205 13 439 438 393228 13 440 1 69 439 196670
     437 440 262205 13 442 437 262205 13 443 433 327809 13
     444 442 443 393228 13 445 1 69 444 196670 441 445
     262205 13 448 433 196670 447 448 262205 13 450 431 196670
     449 450 262205 13 452 437 196670 451 452 262205 13 454
     441 196670 453 454 262205 6 456 357 196670 455 456 262205
     6 458 388 196670 457 458 262205 13 460 426 196670 459
     460 262205 13 462 428 196670 461 462 262205 6 464 401
     196670 463 464 262205 13 466 391 196670 465 466 262205 13
     468 405 196670 467 468 262205 6 470 358 196670 469 470
     1048633 13 471 48 447 449 451 453 455 457 459 461
     463 465 467 469 196670 446 471 262205 13 473 446 196670
     472 473 262205 13 476 472 327745 34 477 362 58 262205
     6 478 477 327761 6 479 476 0 327761 6 480 476
     1 327761 6 481 476 2 458832 7 482 479 480 481
     478 196670 483 482 327737 7 484 11 483 196670 475 484
     65789 65592 327734 7 11 0 9 196663 8 10 131320 12
     262203 33 50 7 262205 7 51 10 524367 13 52 51
     51 0 1 2 458764 13 55 1 26 52 54 196670
     50 55 262205 13 56 50 327745 34 59 10 58 262205
     6 60 59 327761 6 61 56 0 327761 6 62 56
     1 327761 6 63 56 2 458832 7 64 61 62 63
     60 131326 64 65592 327734 13 15 0 14 131320 16 262205
     13 69 68 393228 13 70 1 69 69 131326 70 65592
     327734 13 21 0 19 196663 18 20 131320 22 262203 34
     73 7 262203 33 89 7 262203 33 92 7 262203 34
     97 7 262203 34 109 7 327745 34 78 20 77 262205
     6 79 78 327813 6 80 75 79 327745 34 82 20
     81 262205 6 83 82 327813 6 84 80 83 327745 34
     85 20 81 262205 6 86 85 327813 6 87 84 86
     327809 6 88 74 87 196670 73 88 196670 89 91 262205
     13 95 89 327811 13 96 94 95 196670 92 96 327745
     34 99 92 98 262205 6 100 99 327745 34 102 92
     101 262205 6 103 102 327745 34 105 92 104 262205 6
     106 105 458764 6 107 1 37 103 106 458764 6 108
     1 37 100 107 196670 97 108 262205 6 110 73 327811
     6 111 110 93 327745 34 113 20 112 262205 6 114
     113 327811 6 115 93 114 458764 6 117 1 26 115
     116 327813 6 118 111 117 327809 6 119 93 118 262205
     6 120 73 327811 6 121 120 93 327745 34 123 20
     122 262205 6 124 123 327811 6 125 93 124 458764 6
     126 1 26 125 116 327813 6 127 121 126 327809 6
     128 93 127 327813 6 129 119 128 262205 6 130 97
     327813 6 131 129 130 196670 109 131 327745 33 133 20
     132 262205 13 134 133 262205 6 135 109 327822 13 136
     134 135 131326 136 65592 327734 13 24 0 19 196663 18
     23 131320 25 327745 33 140 23 139 262205 13 141 140
     327745 33 143 23 142 262205 13 144 143 327745 33 145
     23 142 262205 13 146 145 327745 33 147 23 139 262205
     13 148 147 327813 13 149 146 148 327811 13 150 144
     149 327745 34 152 23 81 262205 6 153 152 327813 6
     154 151 153 327811 6 156 154 155 327745 34 157 23
     81 262205 6 158 157 327813 6 159 156 158 393228 6
     160 1 29 159 327822 13 161 150 160 327809 13 162
     141 161 131326 162 65592 327734 6 28 0 26 196663 18
     27 131320 29 262203 34 165 7 262203 34 168 7 262203
     34 171 7 262203 34 178 7 262203 34 193 7 327745
     34 166 27 112 262205 6 167 166 196670 165 167 327745
     34 169 27 122 262205 6 170 169 196670 168 170 327745
     34 173 27 172 262205 6 174 173 327745 34 175 27
     172 262205 6 176 175 327813 6 177 174 176 196670 171
     177 262205 6 179 165 327813 6 180 75 179 262205 6
     181 165 262205 6 182 171 262205 6 183 171 327811 6
     184 93 183 262205 6 185 165 262205 6 186 165 327813
     6 187 185 186 327813 6 188 184 187 327809 6 189
     182 188 393228 6 190 1 31 189 327809 6 191 181
     190 327816 6 192 180 191 196670 178 192 262205 6 194
     168 327813 6 195 75 194 262205 6 196 168 262205 6
     197 171 262205 6 198 171 327811 6 199 93 198 262205
     6 200 168 262205 6 201 168 327813 6 202 200 201
     327813 6 203 199 202 327809 6 204 197 203 393228 6
     205 1 31 204 327809 6 206 196 205 327816 6 207
     195 206 196670 193 207 262205 6 208 178 262205 6 209
     193 327813 6 210 208 209 131326 210 65592 327734 6 31
     0 26 196663 18 30 131320 32 262203 34 213 7 262203
     34 219 7 327745 34 214 30 172 262205 6 215 214
     327745 34 216 30 172 262205 6 217 216 327813 6 218
     215 217 196670 213 218 327745 34 221 30 220 262205 6
     222 221 262205 6 223 213 327813 6 224 222 223 327745
     34 225 30 220 262205 6 226 225 327811 6 227 224
     226 327745 34 228 30 220 262205 6 229 228 327813 6
     230 227 229 327809 6 231 230 93 196670 219 231 262205
     6 232 213 262205 6 234 219 327813 6 235 233 234
     262205 6 236 219 327813 6 237 235 236 327816 6 238
     232 237 131326 238 65592 327734 13 48 0 35 196663 33
     36 196663 33 37 196663 33 38 196663 33 39 196663 34
     40 196663 34 41 196663 33 42 196663 33 43 196663 34
     44 196663 33 45 196663 33 46 196663 34 47 131320 49
     262203 33 241 7 262203 34 252 7 262203 34 258 7
     262203 34 264 7 262203 34 270 7 262203 34 275 7
     262203 34 280 7 262203 18 285 7 262203 33 300 7
     262203 18 301 7 262203 34 304 7 262203 18 305 7
     262203 34 308 7 262203 18 309 7 262203 33 312 7
     262203 18 316 7 262203 33 320 7 262203 33 333 7
     262203 33 343 7 262205 13 242 36 262205 13 243 37
     458764 13 244 1 71 242 243 393228 13 245 1 69
     244 262271 13 246 245 196670 241 246 327745 34 248 241
     101 262205 6 249 248 327813 6 250 249 247 327745 34
     251 241 101 196670 251 250 262205 13 253 37 262205 13
     254 38 327828 6 255 253 254 524300 6 257 1 43
     255 256 93 196670 252 257 262205 13 259 37 262205 13
     260 36 327828 6 261 259 260 393228 6 262 1 4
     261 524300 6 263 1 43 262 256 93 196670 258 263
     262205 13 265 37 262205 13 266 39 327828 6 267 265
     266 524300 6 269 1 43 267 268 93 196670 264 269
     262205 13 271 38 262205 13 272 39 327828 6 273 271
     272 524300 6 274 1 43 273 268 93 196670 270 274
     262205 13 276 36 262205 13 277 39 327828 6 278 276
     277 524300 6 279 1 43 278 268 93 196670 275 279
     262205 13 281 36 262205 13 282 38 327828 6 283 281
     282 524300 6 284 1 43 283 268 93 196670 280 284
     262205 6 286 252 262205 6 287 258 262205 6 288 264
     262205 6 289 270 262205 6 290 275 262205 6 291 280
     262205 6 292 40 262205 6 293 41 262205 13 294 42
     262205 13 295 43 262205 6 296 44 262205 13 297 45
     262205 13 298 46 1048656 17 299 286 287 288 289 290
     291 292 293 294 295 296 297 298 196670 285 299 262205
     17 302 285 196670 301 302 327737 13 303 24 301 196670
     300 303 262205 17 306 285 196670 305 306 327737 6 307
     28 305 196670 304 307 262205 17 310 285 196670 309 310
     327737 6 311 31 309 196670 308 311 262205 13 313 300
     393296 13 314 93 93 93 327811 13 315 314 313 262205
     17 317 285 196670 316 317 327737 13 318 21 316 327813
     13 319 315 318 196670 312 319 262205 13 321 300 262205
     6 322 304 327822 13 323 321 322 262205 6 324 308
     327822 13 325 323 324 262205 6 327 252 327813 6 328
     326 327 262205 6 329 258 327813 6 330 328 329 393296
     13 331 330 330 330 327816 13 332 325 331 196670 320
     332 262205 6 334 252 327822 13 335 94 334 262205 13
     336 312 262205 13 337 320 327809 13 338 336 337 327813
     13 339 335 338 196670 333 339 262205 6 340 47 262205
     13 341 333 327822 13 342 341 340 196670 333 342 327745
     347 348 346 122 262205 7 349 348 524367 13 350 349
     349 0 1 2 196670 343 350 262205 13 351 343 262205
     13 352 333 327809 13 353 352 351 196670 333 353 262205
     13 354 333 131326 354 65592
  }
  NumSpecializationConstants 0
}
)");
vsg::VSG io;
return io.read_cast<vsg::ShaderStage>(str);
};
