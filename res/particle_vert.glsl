#version 330 core
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in float scale;

uniform vec2 cam_pos;
uniform vec2 viewport;
uniform float w_scale;

out vec4 color_modifier;

void main()
{
	color_modifier = in_color;
	gl_PointSize = scale * 20.f / w_scale;

    vec2 new_pos = in_position;

    new_pos.x -= cam_pos.x;
    new_pos.y -= cam_pos.y;

    new_pos.x = (((new_pos.x + (viewport.x / 2)) / viewport.x) * 2) - 1;
    new_pos.y = (((new_pos.y + (viewport.y / 2)) / viewport.y) * 2) - 1;

	gl_Position = vec4(new_pos, 0, 1.0f);
}
