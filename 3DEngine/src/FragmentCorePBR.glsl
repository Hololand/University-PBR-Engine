#version 440
// PBR FRAGMENT SHADER
out vec4 fs_color;

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;
in vec3 vs_tangent;

struct Material
{
	vec3 ambient;
	sampler2D albedoTex;
	sampler2D metalTex;
	sampler2D roughTex;
	sampler2D normTex;
	sampler2D aoTex;
};

struct PointLight
{
	vec3 position;
	float intensity;
	vec3 colour;
	float constant;
	float flinear;
	float quadratic;
};

uniform Material material;
uniform PointLight pointLight;
uniform vec3 cameraPos;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

const float PI = 3.14159265359f;

float DistributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
	denom = PI * denom * denom;
	return a2 / max(denom, 0.0000001f);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f;
	float ggx1 = NdotV / (NdotV * (1.0f - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0f - k) + k);
	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float HdotV, vec3 baseReflectivity)
{
	return baseReflectivity + (1.0f - baseReflectivity) * pow(1.0f - HdotV, 5.0f);
}
// Fresnell schlick equation where higher roughness equates to less fresnel (used for Specular IBL)
vec3 FresnelSchlickRoughness(float HdotV, vec3 baseReflectivity, float roughness)
{
	return baseReflectivity + (max(vec3(1.0f - roughness), baseReflectivity) - baseReflectivity) * pow(1.0f - HdotV, 5.0f);
}
// Custom power function due to some wierd shader compilation issue
vec3 my_pow(vec3 vec, float expt) {
	return vec3(pow(vec.x, expt), pow(vec.y, expt), pow(vec.z, expt));
}

void main()
{
	// Calculate normal, tangent, bitangent
	vec3 normal = normalize(vs_normal);
	vec3 tangent = normalize(vs_tangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(tangent, normal);
	vec3 texNorm = texture(material.normTex, vs_texcoord).rgb;
	texNorm = 2.0 * texNorm - vec3(1.0f);
	// Calculate final normal with respect to normal map
	mat3 TBN = mat3(tangent, bitangent, normal);
	vec3 finalNorm = TBN * texNorm;
	finalNorm = normalize(finalNorm);
	normal = finalNorm;
	// Sample from texture maps, Converting albedo to linear space
	vec3 albedo = my_pow(texture(material.albedoTex, vs_texcoord).rgb, 2.2);
	float metallic = texture(material.metalTex, vs_texcoord).r;
	float roughness = texture(material.roughTex, vs_texcoord).r;

	vec3 N = normalize(normal);
	vec3 V = normalize(cameraPos - vs_position);

	vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);

	vec3 Lo = vec3(0.0f);

	// Per light radiance
	vec3 L = normalize(pointLight.position - vs_position);
	vec3 H = normalize(V + L);
	float distance = length(pointLight.position - vs_position);
	float attenuation = pointLight.intensity / (distance * distance);
	vec3 radiance = pointLight.colour * attenuation;
		
	float NdotV = max(dot(N, V), 0.0000001f);
	float NdotL = max(dot(N, L), 0.0000001f);
	float HdotV = max(dot(H, V), 0.0f);
	float NdotH = max(dot(N, H), 0.0f);

	// Cook-Torrance BRDF
	float D = DistributionGGX(NdotH, roughness);
	float G = GeometrySmith(NdotV, NdotL, roughness);
	vec3 F = FresnelSchlick(HdotV, baseReflectivity);

	vec3 specular = D * G * F;
	specular /= 1.0 * NdotV * NdotL;

	vec3 kD = vec3(1.0f) - F;
	// kD must be 1.0 - F (ks) to abide by the rules of energy conservation. Diffuse and specular cannot be greater than 1.0 where 1.0 is the incoming light
	kD *= 1.0f - metallic, vs_texcoord;
	// add outgoing radiance Lo
	Lo += (kD * albedo / PI + specular) * radiance * NdotL;

	// Get ambient light from irradiance map
	vec3 F2 = FresnelSchlickRoughness(NdotV, baseReflectivity, roughness);
	vec3 kD2 = (1.0f - F2) * (1.0f - metallic);
	vec3 diffuse = texture(irradianceMap, N).rgb * albedo * kD2;

	// calculate specular light
	// combine sample of prefiltered map and BRDF look up texture using the split sum approximation
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, reflect(-V, N), roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specular2 = prefilteredColor * (F * brdf.r + brdf.g);

	vec3 ambient = diffuse + specular2;
	vec3 colour = ambient + Lo;
	// HDR tonemapping
	colour = colour / (colour + vec3(1.0f));
	// Gamma correction
	colour = pow(colour, vec3(1.0f / 2.2f));
	fs_color = vec4(colour, 1.0);
}