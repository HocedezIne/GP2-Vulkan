#version 450

layout(push_constant)uniform PushConstants{
    mat4 model;
} mesh;

layout(set =0,binding = 0) uniform GP2_ViewProjection {
    mat4 view;
    mat4 proj;
} vp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vp.proj * vp.view * mesh.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}