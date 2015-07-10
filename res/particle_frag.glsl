#version 330 core
in vec4 color_modifier;
out vec4 color;

uniform sampler2D uf_Texture;

void main () 
{
    //color = texture2D (uf_Texture, gl_PointCoord.xy);
    // Took out color_modifier
    color = vec4(0.2f, 0.8f, 0.2f,  1);
}
