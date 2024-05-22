#version 450

// ------------------ LAYOUT ------------------------------

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D roughnessSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec3 fragViewDirection;

layout(location = 0) out vec4 outColor;

// ------------------ HELPERS ------------------------------

vec4 Lambert(vec4 kd, vec4 cd)
{
	return (cd * kd) / 3.14159265358979323846f ;
}

vec4 FresnelFunction_Schlick(vec3 h, vec3 v, vec4 f0)
{
	const float schlick = 1 - dot(h,v);
	return f0 + (vec4(1,1,1, 0) - f0) * (schlick * schlick * schlick * schlick * schlick);
}

float NormalDistribution_GGX(vec3 n, const vec3 h, float roughness)
{
	const float a =  roughness * roughness;
	const float dotNH = dot(n,h) * dot(n,h);

	return a / (3.14159265358979323846f * ( dotNH*(a-1.f) +1.f) * ( dotNH*(a-1.f) +1.f) );
}

float GeometryFunction_SchlickGGX(vec3 n, vec3 v, float roughness)
{
	const float dotNV = dot(n,v);
	return dotNV / (dotNV * (1-roughness) + roughness);
}

float GeometryFunction_Smith(vec3 n, vec3 v, vec3 l, float roughness)
{
	const float remappedK = ((roughness + 1) * (roughness+1)) / 8;
	return GeometryFunction_SchlickGGX(n,v,remappedK) * GeometryFunction_SchlickGGX(n,l,remappedK);
}

// ------------------ MAIN -------------------------------------

void main() {
	// values from maps + light direction
    vec4 diffuseMap = texture(diffuseSampler, fragTexCoord);
    vec4 normalMap = texture(normalSampler, fragTexCoord);
    vec4 roughnessMap = texture(roughnessSampler, fragTexCoord);
	const vec3 lightDirection = vec3(0.577f, -0.577, 0.577f);

	// calculate normals
	const vec3 binormal = cross(fragNormal, fragTangent);
	const mat3 tangentSpaceAxis = mat3(fragTangent, binormal, fragNormal);
	vec3 normal = (2.f * vec3(normalMap) - vec3(1.f,1.f,1.f));
	normal = tangentSpaceAxis * normal;

	const float observedArea = dot(normal, lightDirection);
	if( observedArea <= 0.f) outColor = vec4(0.f,0.f,0.f,0.f);
	else 
	{
		const vec4 f0 = (abs(roughnessMap.x-0) <  1.19209290e-07f) ? vec4(0.04f, 0.04f, 0.04f, 1.f) : diffuseMap;

		const vec3 h = normalize(fragViewDirection + lightDirection);

		const vec4 F = FresnelFunction_Schlick(h, fragViewDirection, f0);
		const float D = NormalDistribution_GGX(normal, h, roughnessMap.x*roughnessMap.x);
		const float G = GeometryFunction_Smith(normal, fragViewDirection, lightDirection, roughnessMap.x*roughnessMap.x);

		const float divisor = 4 * dot(fragViewDirection, normal) * dot(lightDirection, normal);
		vec4 specular = D*F*G;
		specular /= divisor;

		const vec4 kd = (abs(roughnessMap.x-0) <  1.19209290e-07f) ? vec4( 1.f,1.f,1.f, 1.f ) - F : vec4( 0.f,0.f,0.f, 0.f );
		const vec4 diffuse = Lambert(kd, diffuseMap);

		outColor = vec4(observedArea, observedArea, observedArea, 1.f);
	}
}