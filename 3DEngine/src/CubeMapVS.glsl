#version 440

layout(location = 0) in vec3 aPos;

out vec3 position;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    position = aPos;
    gl_Position = projection * view * vec4(position, 1.0);
}
