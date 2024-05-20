#version 450

layout(push_constant)uniform PushConstants{
    mat4 model;
} mesh;

layout(set =0,binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;

void main() {
    gl_Position = ubo.proj * ubo.view * mesh.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragNormal = mat3(mesh.model) * normalize(inNormal);
}