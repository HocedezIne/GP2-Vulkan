#version 450

// ------------------ LAYOUT ------------------------------

layout(push_constant)uniform PushConstants{
    layout(offset=64) int mode;
} rendermode;

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D metalnessSampler;
layout(binding = 4) uniform sampler2D roughnessSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec3 fragViewDirection;

layout(location = 0) out vec4 outColor;

// ------------------ HELPERS ------------------------------

vec3 Lambert(vec3 kd, vec3 cd)
{
	return (cd * kd) / 3.14159265358979323846f ;
}

vec3 FresnelFunction_Schlick(vec3 h, vec3 v, vec3 f0)
{
	const float schlick = 1 - dot(h,v);
	return clamp(f0 + (1.f - f0) * pow(schlick, 5), 0.f, 1.f);
}

float NormalDistribution_GGX(vec3 n, const vec3 h, float roughness)
{
	const float a =  roughness * roughness;
	const float dotNH = dot(n,h) * dot(n,h);

	const float denom = dotNH * a + (1.0 - dotNH);
	return a / (3.14159265358979323846f * pow(denom, 2) );
}

float GeometryFunction_SchlickGGX(vec3 n, vec3 v, float roughness)
{
	const float dotNV = dot(n,v);
	return (dotNV*2) / ((dotNV) + sqrt(roughness + (1-roughness) * (dotNV*dotNV)));
}

float GeometryFunction_Smith(vec3 n, vec3 v, vec3 l, float roughness)
{
	const float remappedK = ((roughness + 1) * (roughness+1)) / 8;
	return GeometryFunction_SchlickGGX(n,v,remappedK) * GeometryFunction_SchlickGGX(n,l,remappedK);
}

// ------------------ MAIN -------------------------------------

void main() {
	// values from maps + light direction
    vec3 albedo = texture(diffuseSampler, fragTexCoord).rgb;

	vec3 normalMap = texture(normalSampler, fragTexCoord).rgb;
    float roughnessValue = clamp(texture(roughnessSampler, fragTexCoord).x, 0.01f, 0.99f);
	float metalnessValue = texture(metalnessSampler, fragTexCoord).x;

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

	const vec3 lightDirection = normalize(vec3(0.577f, 0.577f, 0.577f));
	const vec3 radiance = vec3(7.f,7.f,7.f);

	const float observedArea = dot(normal, lightDirection);
	if( observedArea <= 0.f) 
	{
		outColor = vec4(0.f,0.f,0.f,0.f);
		return;
	}

	const vec3 f0 = ( abs(metalnessValue - 0) < 1e-5f ) ? vec3(0.04f, 0.04f, 0.04f) : albedo;

	const vec3 h = normalize(fragViewDirection + lightDirection);

	const vec3 F = FresnelFunction_Schlick(h, fragViewDirection, f0);
	const float D = NormalDistribution_GGX(normal, h, roughnessValue*roughnessValue);
	const float G = GeometryFunction_Smith(normal, fragViewDirection, lightDirection, roughnessValue*roughnessValue);

	const float divisor = 4* dot(fragViewDirection, normal) * dot(lightDirection, normal);
	vec3 specular = F*D*G;
	specular /= divisor;

	if(rendermode.mode == 3)
	{
		outColor = vec4(specular, 1.f);
		return;
	}

	vec3 kd = ( abs(metalnessValue - 0) < 1e-5f ) ? 1.f - F : vec3(0.f);
	const vec3 diffuse = Lambert(kd, albedo);

	if(rendermode.mode == 1)
	{
		outColor = vec4(diffuse, 1.f);
		return;
	}

	outColor = vec4(radiance * (diffuse + specular) * observedArea ,0.f);
}