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
#extension GL_ARB_separate_shader_objects : enable\n\
\n\
layout(binding = 0) uniform sampler2D texSampler;\n\
\n\
layout(location = 0) in vec4 fragColor;\n\
layout(location = 1) in vec2 fragTexCoord;\n\
\n\
layout(location = 0) out vec4 outColor;\n\
\n\
void main() {\n\
    outColor = vec4(fragColor.rgb, fragColor.a * texture(texSampler, fragTexCoord).r);\n\
    //outColor = fragColor;\n\
    if (outColor.a == 0.0) discard;\n\
}\n\
\"\n\
    SPIRVSize 278\n\
    SPIRV 119734787 65536 524298 46 0 131089 1 393227 1 1280527431 1685353262 808793134\n\
     0 196622 0 1 524303 4 4 1852399981 0 9 11 27\n\
     196624 4 7 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331\n\
     1868526181 1667590754 29556 262149 4 1852399981 0 327685 9 1131705711 1919904879 0\n\
     327685 11 1734439526 1869377347 114 327685 23 1400399220 1819307361 29285 393221 27\n\
     1734439526 1131963732 1685221231 0 262215 9 30 0 262215 11 30 0\n\
     262215 23 34 0 262215 23 33 0 262215 27 30 1\n\
     131091 2 196641 3 2 196630 6 32 262167 7 6 4\n\
     262176 8 3 7 262203 8 9 3 262176 10 1 7\n\
     262203 10 11 1 262167 12 6 3 262165 15 32 0\n\
     262187 15 16 3 262176 17 1 6 589849 20 6 1\n\
     0 0 0 1 0 196635 21 20 262176 22 0 21\n\
     262203 22 23 0 262167 25 6 2 262176 26 1 25\n\
     262203 26 27 1 262187 15 30 0 262176 37 3 6\n\
     262187 6 40 0 131092 41 327734 2 4 0 3 131320\n\
     5 262205 7 13 11 524367 12 14 13 13 0 1\n\
     2 327745 17 18 11 16 262205 6 19 18 262205 21\n\
     24 23 262205 25 28 27 327767 7 29 24 28 327761\n\
     6 31 29 0 327813 6 32 19 31 327761 6 33\n\
     14 0 327761 6 34 14 1 327761 6 35 14 2\n\
     458832 7 36 33 34 35 32 196670 9 36 327745 37\n\
     38 9 16 262205 6 39 38 327860 41 42 39 40\n\
     196855 44 0 262394 42 43 44 131320 43 65788 131320 44\n\
     65789 65592\n\
  }\n\
  NumSpecializationConstants 0\n\
}\n\
");
vsg::ReaderWriter_vsg io;
return io.read_cast<vsg::ShaderStage>(str);
};
