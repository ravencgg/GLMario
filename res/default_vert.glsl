#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 in_tex_coord;
uniform float time;
uniform mat4 mvp;
out vec4 color_modifier;
out vec2 frag_tex_coord;

void main()
{
 	color_modifier = color * time;
 	frag_tex_coord = in_tex_coord;

 	vec4 new_position = mvp * vec4(position, 1.0f);
	gl_Position = new_position; // mvp * vec4(position, 1.0f);
}