#version 440
layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 position;

void main()
{
	position = aPos;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(position, 1.0);

	gl_Position = clipPos.xyww;
}