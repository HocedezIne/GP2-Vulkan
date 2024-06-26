#version 450

layout(push_constant)uniform PushConstants{
    mat4 model;
} mesh;

layout(set =0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * mesh.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}