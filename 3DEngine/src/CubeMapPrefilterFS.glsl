#version 440

out vec4 FragColor;
in vec3 position;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

// Hammersley sequence random number generation helper function
float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Trowbridge-Reitz GGX
vec3 importanceSampleGGX(uint i, uint sampleCount, vec3 N, float roughness)
{
	vec2 hammersley = vec2(float(i) / float(sampleCount), radicalInverse_VdC(i));

	float a = roughness * roughness;

	float phi = 2.0 * PI * hammersley.x;
	float cosTheta = sqrt((1.0 - hammersley.y) / (1.0 + (a * a - 1.0) * hammersley.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	// from spherical coordinates to tangent space cartesian coordinates
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	// fromt tangent space H vector to world space sample vector
	vec3 up = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

void main()
{
	// Use normal as location
	vec3 N = normalize(position);
	vec3 V = N;
	// Samples in hemisphere around normal
	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0f;
	vec3 prefilteredColor = vec3(0.0f);

	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		// Given input normal and roughness return random sample halfway vectors (using hammersly low-discrepancy sequence)
		// Where the higher the roughness the wider the spread
		vec3 H = importanceSampleGGX(i, SAMPLE_COUNT, N, roughness);
		vec3 L = reflect(-V, H);
		float NdotL = max(dot(N, L), 0.0f);

		if (NdotL > 0.0f)
		{
			// determine mip level from roughness
			float D = distributionGGX(N, H, roughness);
			float NdotH = max(dot(N, H), 0.0f);
			float VdotH = max(dot(V, H), 0.0f);
			float pdf = D * NdotH / (4.0f * VdotH) + 0.0001f;

			float resolution = 512.0f; // source cubemap resolution
			float saTexel = 4.0f * PI / (6.0f * resolution * resolution);
			float saSample = 1.0f / (float(SAMPLE_COUNT) * pdf + 0.0001);

			float mipLevel = roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);

			prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;
	FragColor = vec4(prefilteredColor, 1.0f);
}