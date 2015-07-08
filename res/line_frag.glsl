#version 330 core
in vec4 color_modifier;
in vec2 frag_tex_coord;
out vec4 color;
// uniform float time;
uniform mat4 mvp;
uniform sampler2D ourTexture;

void main()
{
	color = texture(ourTexture, frag_tex_coord);// + vec4(1.0f, 1.0f, 0f, 0f);// * ((sin(time) + 1) / 2);
}
