#version 330 core
in vec2 frag_tex_coord;
in vec4 color_modifier;
out vec4 color;
uniform sampler2D ourTexture;

void main()
{
    color = texture(ourTexture, frag_tex_coord);
    color *= color_modifier;
}
