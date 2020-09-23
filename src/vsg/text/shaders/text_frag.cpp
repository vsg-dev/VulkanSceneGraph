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
    outColor = texture(texSampler, fragTexCoord) * fragColor;\n\
    if (outColor.a == 0.0) discard;\n\
}\n\
\"\n\
    SPIRVSize 222\n\
    SPIRV 119734787 65536 524298 35 0 131089 1 393227 1 1280527431 1685353262 808793134\n\
     0 196622 0 1 524303 4 4 1852399981 0 9 17 21\n\
     196624 4 7 196611 2 450 589828 1096764487 1935622738 1918988389 1600484449 1684105331\n\
     1868526181 1667590754 29556 262149 4 1852399981 0 327685 9 1131705711 1919904879 0\n\
     327685 13 1400399220 1819307361 29285 393221 17 1734439526 1131963732 1685221231 0 327685\n\
     21 1734439526 1869377347 114 262215 9 30 0 262215 13 34 0\n\
     262215 13 33 0 262215 17 30 1 262215 21 30 0\n\
     131091 2 196641 3 2 196630 6 32 262167 7 6 4\n\
     262176 8 3 7 262203 8 9 3 589849 10 6 1\n\
     0 0 0 1 0 196635 11 10 262176 12 0 11\n\
     262203 12 13 0 262167 15 6 2 262176 16 1 15\n\
     262203 16 17 1 262176 20 1 7 262203 20 21 1\n\
     262165 24 32 0 262187 24 25 3 262176 26 3 6\n\
     262187 6 29 0 131092 30 327734 2 4 0 3 131320\n\
     5 262205 11 14 13 262205 15 18 17 327767 7 19\n\
     14 18 262205 7 22 21 327813 7 23 19 22 196670\n\
     9 23 327745 26 27 9 25 262205 6 28 27 327860\n\
     30 31 28 29 196855 33 0 262394 31 32 33 131320\n\
     32 65788 131320 33 65789 65592\n\
  }\n\
  NumSpecializationConstants 0\n\
}\n\
");
vsg::ReaderWriter_vsg io;
return io.read_cast<vsg::ShaderStage>(str);
};
