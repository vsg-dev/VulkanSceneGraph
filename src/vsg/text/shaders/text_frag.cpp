#include <vsg/io/VSG.h>
static auto text_frag = []() {std::istringstream str(
R"(#vsga 0.2.13
Root id=1 vsg::ShaderStage
{
  NumUserObjects 0
  stage 16
  entryPointName "main"
  module id=2 vsg::ShaderModule
  {
    NumUserObjects 0
    Source "#version 450

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 outlineColor;
layout(location = 2) in float outlineWidth;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec2 glyph_alpha(vec2 texcoord, vec2 dx, vec2 dy)
{
    float lod = textureQueryLod(texSampler, texcoord).x;
    float innerCutOff = 0.0;
    if (lod>0.0) innerCutOff = lod * 0.03;

    float scale = 1.0;
    float distance_from_edge = (textureGrad(texSampler, texcoord, dx, dy).r);

    float d_distance_dx = dFdx(distance_from_edge) * scale;
    float d_distance_dy = dFdy(distance_from_edge) * scale;

    float delta = sqrt(d_distance_dx * d_distance_dx + d_distance_dy * d_distance_dy);

    float min_distance_from_edge = distance_from_edge - delta;
    float max_distance_from_edge = distance_from_edge + delta;

    //min_distance_from_edge += 0.0;
    min_distance_from_edge += innerCutOff;
    float inner_alpha = 0.0;
    if (min_distance_from_edge >= 0.0) inner_alpha = 1.0;
    else if (max_distance_from_edge >= 0.0) inner_alpha = max_distance_from_edge/(max_distance_from_edge-min_distance_from_edge);

    min_distance_from_edge += outlineWidth;
    float outer_alpha = 0.0;
    if (min_distance_from_edge >= 0.0) outer_alpha = 1.0;
    else if (max_distance_from_edge >= 0.0) outer_alpha = max_distance_from_edge/(max_distance_from_edge-min_distance_from_edge);

    return vec2(inner_alpha, outer_alpha);
}

vec2 sampled_glyph_alpha_grid(vec2 texcoord)
{
    float lod = textureQueryLod(texSampler, texcoord).x;
    vec2 dx = dFdx(texcoord);
    vec2 dy = dFdy(texcoord);

    if (lod<=0.0) return glyph_alpha(texcoord, dx, dy);

    float area = length(dx) * length(dy);
    float average_side = sqrt(area) / (1.0 + lod);
    float num_x = ceil(length(dx) / average_side);
    float num_y = ceil(length(dy) / average_side);

    vec2 interval_dx = dx / num_x;
    vec2 interval_dy = dy / num_y;

    vec2 total_alpha = vec2(0.0, 0.0);
    vec2 tc_row_start = texcoord - dx*0.5 - dy*0.5;
    for(float r = 0; r<num_y; ++r)
    {
        vec2 tc = tc_row_start;
        tc_row_start = tc_row_start + interval_dy;

        for(float c = 0; c<num_x; ++c)
        {
            total_alpha = total_alpha + glyph_alpha(tc, interval_dx, interval_dy);

            tc = tc + interval_dx;
        }
    }

    return total_alpha / (num_x * num_y);
}

void main()
{
    vec2 alphas = sampled_glyph_alpha_grid(fragTexCoord);

    if (alphas[1]>0.0)
    {
        vec4 glyph = vec4(fragColor.rgb, fragColor.a * alphas[0]);
        vec4 outline = vec4(outlineColor.rgb, outlineColor.a * alphas[1]);
        outColor = mix(outline, glyph, glyph.a);
    }
    else
    {
        outColor = vec4(fragColor.rgb,  fragColor.a * alphas[0]);
    }

    if (outColor.a == 0.0) discard;
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
