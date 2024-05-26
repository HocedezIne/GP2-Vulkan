#version 450

// ------------------ LAYOUT ------------------------------

layout(push_constant)uniform PushConstants{
    mat4 model;
} mesh;

layout(set =0,binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTangent;
layout(location = 3) out vec3 fragViewDirection;

// ------------------ MAIN ----------------------------------

void main() {
    gl_Position = ubo.proj * ubo.view * mesh.model * vec4(inPosition, 1.0);

    fragViewDirection = normalize(vec3(ubo.view[2][0], ubo.view[2][1], ubo.view[2][2]) - vec3(gl_Position));

    fragTexCoord = inTexCoord;
    fragNormal = normalize(mat3(mesh.model) * inNormal);
    fragTangent = normalize(mat3(mesh.model) * inTangent);
}