#include <vsg/io/ReaderWriter_vsg.h>
static auto text_frag = []() {std::istringstream str(
"#vsga 0.0.2\n\
Root id=1 vsg::ShaderStage\n\
{\n\
  NumUserObjects 0\n\
  Stage 16\n\
  EntryPoint \"main\"\n\
  ShaderModule id=2 vsg::ShaderModule\n\
  {\n\
    NumUserObjects 0\n\
    Source \"#version 450\n\
\n\
layout(binding = 0) uniform sampler2D texSampler;\n\
\n\
layout(location = 0) in vec4 fragColor;\n\
layout(location = 1) in vec2 fragTexCoord;\n\
\n\
layout(location = 0) out vec4 outColor;\n\
\n\
//#define GREYCALE\n\
\n\
void main()\n\
{\n\
#if GREYSCALE\n\
#if 0\n\
    float lod = textureQueryLod(texSampler, fragTexCoord).x;\n\
    float start_cutoff = 2.0;\n\
    float end_cutoff = 8.0;\n\
    float alpha_multiplier = mix(1.0, 0.0, (lod-start_cutoff)/(end_cutoff-start_cutoff));\n\
#else\n\
    float alpha_multiplier = 1.0;\n\
#endif\n\
    outColor = vec4(fragColor.rgb, fragColor.a * texture(texSampler, fragTexCoord).r * alpha_multiplier);\n\
#else\n\
\n\
    float distance_from_edge = (texture(texSampler, fragTexCoord).r - 0.5);\n\
    float alpha_multiplier = 0.0;\n\
#if 1\n\
    float blend_distance = -0.05;\n\
    if (distance_from_edge >= 0.0) alpha_multiplier = 1.0;\n\
    else if (distance_from_edge >= blend_distance)\n\
    {\n\
        float boundary_ratio = (distance_from_edge) / blend_distance;\n\
        alpha_multiplier = smoothstep(1.0, 0.0, boundary_ratio);\n\
    }\n\
#else\n\
    if (distance_from_edge >= 0.0) alpha_multiplier = 1.0;\n\
#endif\n\
\n\
    outColor = vec4(fragColor.rgb, fragColor.a * alpha_multiplier );\n\
#endif\n\
\n\
    //outColor = fragColor;\n\
    if (outColor.a == 0.0) discard;\n\
}\n\
\"\n\
    SPIRVSize 431\n\
    SPIRV 119734787 65536 524298 71 0 131089 1 393227 1 1280527431 1685353262 808793134\n\
     0 196622 0 1 524303 4 4 1852399981 0 16 48 50\n\
     196624 4 7 196611 2 450 262149 4 1852399981 0 458757 8\n\
     1953720676 1701015137 1869768287 1684365165 25959 327685 12 1400399220 1819307361 29285 393221 16\n\
     1734439526 1131963732 1685221231 0 458757 25 1752198241 1970102113 1885959276 1919248748 0 393221\n\
     27 1852140642 1768185700 1851880563 25955 393221 41 1853189986 2037539172 1952543327 28521 327685\n\
     48 1131705711 1919904879 0 327685 50 1734439526 1869377347 114 262215 12 34\n\
     0 262215 12 33 0 262215 16 30 1 262215 48 30\n\
     0 262215 50 30 0 131091 2 196641 3 2 196630 6\n\
     32 262176 7 7 6 589849 9 6 1 0 0 0\n\
     1 0 196635 10 9 262176 11 0 10 262203 11 12\n\
     0 262167 14 6 2 262176 15 1 14 262203 15 16\n\
     1 262167 18 6 4 262165 20 32 0 262187 20 21\n\
     0 262187 6 23 1056964608 262187 6 26 0 262187 6 28\n\
     3175926989 131092 30 262187 6 34 1065353216 262176 47 3 18 262203\n\
     47 48 3 262176 49 1 18 262203 49 50 1 262167\n\
     51 6 3 262187 20 54 3 262176 55 1 6 262176\n\
     64 3 6 327734 2 4 0 3 131320 5 262203 7\n\
     8 7 262203 7 25 7 262203 7 27 7 262203 7\n\
     41 7 262205 10 13 12 262205 14 17 16 327767 18\n\
     19 13 17 327761 6 22 19 0 327811 6 24 22\n\
     23 196670 8 24 196670 25 26 196670 27 28 262205 6\n\
     29 8 327870 30 31 29 26 196855 33 0 262394 31\n\
     32 35 131320 32 196670 25 34 131321 33 131320 35 262205\n\
     6 36 8 262205 6 37 27 327870 30 38 36 37\n\
     196855 40 0 262394 38 39 40 131320 39 262205 6 42\n\
     8 262205 6 43 27 327816 6 44 42 43 196670 41\n\
     44 262205 6 45 41 524300 6 46 1 49 34 26\n\
     45 196670 25 46 131321 40 131320 40 131321 33 131320 33\n\
     262205 18 52 50 524367 51 53 52 52 0 1 2\n\
     327745 55 56 50 54 262205 6 57 56 262205 6 58\n\
     25 327813 6 59 57 58 327761 6 60 53 0 327761\n\
     6 61 53 1 327761 6 62 53 2 458832 18 63\n\
     60 61 62 59 196670 48 63 327745 64 65 48 54\n\
     262205 6 66 65 327860 30 67 66 26 196855 69 0\n\
     262394 67 68 69 131320 68 65788 131320 69 65789 65592\n\
  }\n\
  NumSpecializationConstants 0\n\
}\n\
");
vsg::ReaderWriter_vsg io;
return io.read_cast<vsg::ShaderStage>(str);
};
