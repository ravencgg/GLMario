#version 330 core
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in float scale;

uniform mat4 mvp;
out vec4 color_modifier;

void main()
{
	gl_Position = mvp * vec4(in_position, -1.0f, 1.0f);
	color_modifier = in_color;
	gl_PointSize = scale * 1.f;
}