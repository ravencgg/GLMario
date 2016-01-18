#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 in_tex_coord;

uniform vec2 cam_pos;
uniform vec2 viewport;

out vec2 frag_tex_coord;

void main()
{
    vec2 new_pos = position;

    new_pos.x -= cam_pos.x;
    new_pos.y -= cam_pos.y;

    new_pos.x = (new_pos.x * 2.f / viewport.x);
    new_pos.y = (new_pos.y * 2.f / viewport.y);

	gl_Position = vec4(new_pos, 0, 1.0f);
 	frag_tex_coord = in_tex_coord;
}
