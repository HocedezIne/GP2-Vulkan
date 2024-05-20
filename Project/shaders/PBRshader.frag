#version 450

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    const vec4 f0 = (abs(10-0) >  1.19209290e-07f) ? vec4(0.04f, 0.04f, 0.04f, 0.f) : texture(diffuseSampler, fragTexCoord);

    vec4 x = texture(diffuseSampler, fragTexCoord);
    vec4 y = texture(normalSampler, fragTexCoord);

    outColor = vec4(x.x, y.y, y.z, x.w);
}