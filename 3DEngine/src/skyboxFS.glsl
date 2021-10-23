#version 440
out vec4 FragColor;
in vec3 position;

uniform samplerCube environmentMap;

void main()
{
    vec3 envColor = texture(environmentMap, position).rgb;

    // HDR tonemapping and gamma correction
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0 / 2.2));

    FragColor = vec4(envColor, 1.0);
}