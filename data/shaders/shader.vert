#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform ProjectionMatrix { mat4 matrix; } projection;
layout(set = 0, binding = 1) uniform ViewMatrix { mat4 matrix; } view;
layout(set = 0, binding = 2) uniform ModelMatrix { mat4 matrix; } model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = projection.matrix * view.matrix * model.matrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}