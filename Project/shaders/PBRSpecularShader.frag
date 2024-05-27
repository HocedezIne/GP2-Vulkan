#version 450

// ------------------ LAYOUT ------------------------------

layout(push_constant)uniform PushConstants{
    layout(offset=64) int mode;
} rendermode;

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D glossSampler;
layout(binding = 4) uniform sampler2D specularSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec3 fragViewDirection;

layout(location = 0) out vec4 outColor;

// ------------------ HELPERS ------------------------------

vec3 Lambert(vec3 kd, vec3 cd)
{
	return (cd * kd) / 3.14159265358979323846f;
}

vec3 Phong(vec3 lightDir, float reflection, float exponent, vec3 v, vec3 n)
{
	float d = dot(n, lightDir);
	vec3 reflect = lightDir - (2 *  d * n);
	float cosAlpha = max(dot(reflect, v), 0.f);
	float specular = reflection * pow(cosAlpha, exponent);
	return vec3(specular, specular, specular);
}

// ------------------ MAIN -------------------------------------

void main() {
	// values from maps + light direction
    vec3 albedo = texture(diffuseSampler, fragTexCoord).rgb;
	if(rendermode.mode == 1)
	{
		outColor = vec4(albedo, 1.f);
		return;
	}

    vec3 normalMap = texture(normalSampler, fragTexCoord).rgb;
    float glossValue = texture(glossSampler, fragTexCoord).x;
    float specularValue = texture(specularSampler, fragTexCoord).x;

	// calculate normals
	const vec3 binormal = cross(fragNormal, fragTangent);
	const mat3 tangentSpaceAxis = mat3(fragTangent, binormal, fragNormal);
	vec3 normal = 2.f * normalMap - 1.f;
	normal = normalize(tangentSpaceAxis * normal);

	if(rendermode.mode == 2)
	{
		outColor = vec4(normal, 1.f);
		return;
	}

	const vec3 lightDirection = vec3(0.577f, 0.577, 0.577f);
	const vec3 radiance = vec3(7.f,7.f,7.f);

	const float observedArea = dot(normal, lightDirection);
	if( observedArea <= 0.f) 
	{
		outColor = vec4(0.f,0.f,0.f,0.f);
		return;
	}

	vec3 phong = Phong(lightDirection, specularValue, glossValue * 25.f, -fragViewDirection, normal);
	if(rendermode.mode == 3)
	{
		outColor = vec4(phong, 1.f);
		return;
	}

	vec3 lambert = Lambert(radiance, albedo);

	outColor = vec4( (lambert + phong + vec3(0.03f, 0.03f, 0.03f)) * observedArea, 1.f);
}