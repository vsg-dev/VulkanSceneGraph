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
void main()\n\
{\n\
#if 1\n\
    float lod = textureQueryLod(texSampler, fragTexCoord).x;\n\
    float start_cutoff = 2.0;\n\
    float end_cutoff = 8.0;\n\
    float alpha_multiplier = mix(1.0, 0.0, (lod-start_cutoff)/(end_cutoff-start_cutoff));\n\
#else\n\
    float alpha_multiplier = 1.0;\n\
#endif\n\
    outColor = vec4(fragColor.rgb, fragColor.a * texture(texSampler, fragTexCoord).r * alpha_multiplier);\n\
    //outColor = fragColor;\n\
    if (outColor.a == 0.0) discard;\n\
}\n\
\"\n\
    SPIRVSize 402\n\
    SPIRV 119734787 65536 524298 68 0 131089 1 131089 50 393227 1 1280527431\n\
     1685353262 808793134 0 196622 0 1 524303 4 4 1852399981 0 16\n\
     39 41 196624 4 7 196611 2 450 262149 4 1852399981 0\n\
     196613 8 6582124 327685 12 1400399220 1819307361 29285 393221 16 1734439526 1131963732\n\
     1685221231 0 393221 22 1918989427 1969446772 1717989236 0 327685 24 1600417381 1869903203\n\
     26214 458757 26 1752198241 1970102113 1885959276 1919248748 0 327685 39 1131705711 1919904879\n\
     0 327685 41 1734439526 1869377347 114 262215 12 34 0 262215 12\n\
     33 0 262215 16 30 1 262215 39 30 0 262215 41\n\
     30 0 131091 2 196641 3 2 196630 6 32 262176 7\n\
     7 6 589849 9 6 1 0 0 0 1 0 196635\n\
     10 9 262176 11 0 10 262203 11 12 0 262167 14\n\
     6 2 262176 15 1 14 262203 15 16 1 262165 19\n\
     32 0 262187 19 20 0 262187 6 23 1073741824 262187 6\n\
     25 1090519040 262187 6 27 1065353216 262187 6 28 0 262167 37\n\
     6 4 262176 38 3 37 262203 38 39 3 262176 40\n\
     1 37 262203 40 41 1 262167 42 6 3 262187 19\n\
     45 3 262176 46 1 6 262176 60 3 6 131092 63\n\
     327734 2 4 0 3 131320 5 262203 7 8 7 262203\n\
     7 22 7 262203 7 24 7 262203 7 26 7 262205\n\
     10 13 12 262205 14 17 16 327785 14 18 13 17\n\
     327761 6 21 18 0 196670 8 21 196670 22 23 196670\n\
     24 25 262205 6 29 8 262205 6 30 22 327811 6\n\
     31 29 30 262205 6 32 24 262205 6 33 22 327811\n\
     6 34 32 33 327816 6 35 31 34 524300 6 36\n\
     1 46 27 28 35 196670 26 36 262205 37 43 41\n\
     524367 42 44 43 43 0 1 2 327745 46 47 41\n\
     45 262205 6 48 47 262205 10 49 12 262205 14 50\n\
     16 327767 37 51 49 50 327761 6 52 51 0 327813\n\
     6 53 48 52 262205 6 54 26 327813 6 55 53\n\
     54 327761 6 56 44 0 327761 6 57 44 1 327761\n\
     6 58 44 2 458832 37 59 56 57 58 55 196670\n\
     39 59 327745 60 61 39 45 262205 6 62 61 327860\n\
     63 64 62 28 196855 66 0 262394 64 65 66 131320\n\
     65 65788 131320 66 65789 65592\n\
  }\n\
  NumSpecializationConstants 0\n\
}\n\
");
vsg::ReaderWriter_vsg io;
return io.read_cast<vsg::ShaderStage>(str);
};
