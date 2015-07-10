#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 in_tex_coord;
uniform float time;
uniform mat4 mvp;
uniform vec4 color_in;
out vec2 frag_tex_coord;
out vec4 color_modifier;

void main()
{
 	frag_tex_coord = in_tex_coord;
 	color_modifier = color_in;
	gl_Position = mvp * vec4(position, 1.0f);
}