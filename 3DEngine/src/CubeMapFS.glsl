#version 440

out vec4 FragColor;

in vec3 position;
// HDR image
uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
// Get UV coordinate from Sphere Sample
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
// Use UV coordinate to sample from HDR
void main()
{
    vec2 uv = SampleSphericalMap(normalize(position));
    vec3 color = texture(equirectangularMap, uv).rgb;

    FragColor = vec4(color, 1.0);
}
