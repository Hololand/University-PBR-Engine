#version 440

out vec4 FragColor;
in vec3 position;

// Cubemap
uniform samplerCube environmentMap;

const float PI = 3.14159265359f;

void main()
{
	// Use position as vector for fragment. Essentially a normal of the tangent surface from the origin.
	vec3 N = normalize(position);
	vec3 irradiance = vec3(0.0);
	// Caclulate tangent from origin
	vec3 right = cross(vec3(0.0f, 1.0f, 0.0f), N);
	// Calculate bitangent
	vec3 up = cross(N, right);

	float sampleDelta = 0.025f;
	float nrSamples = 0.0f;
	for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta) {
		for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
		{
			// Spherical to cartesian.
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
			// Tangent space to world space
			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta); // The further a sample is from the center of hemisphere the less influence it has
			nrSamples++;
		}
	}
	irradiance *= PI / nrSamples;
	FragColor = vec4(irradiance, 1.0f);
}
