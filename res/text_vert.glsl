#version 330 core
layout(location = 0) in vec4 vertex;
out vec2 frag_tex_coord;

void main()
{
    frag_tex_coord = vertex.zw;
    gl_Position = vec4(vertex.xy, 0, 1.0f);
}
