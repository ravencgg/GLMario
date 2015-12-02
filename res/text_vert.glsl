#version 330 core
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 vertex_color;

out vec4 color_modifier;
out vec2 frag_tex_coord;

uniform vec2 cam_pos;
uniform vec2 viewport;

void main()
{
    color_modifier = vertex_color;

    frag_tex_coord = vertex.zw;
    //gl_Position = vec4(vertex.xy, 0, 1.0f);
    vec2 new_pos = vertex.xy;
    new_pos.x -= cam_pos.x;
    new_pos.y -= cam_pos.y;
    new_pos.x = (new_pos.x * 2.f / viewport.x);
    new_pos.y = (new_pos.y * 2.f / viewport.y);
    gl_Position = vec4(new_pos, 0, 1.0f);
}
