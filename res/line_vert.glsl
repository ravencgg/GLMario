#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
uniform mat4 mvp;
out vec4 oColor;

void main()
{
    oColor = color;
    vec3 new_pos = vec3(position, -1.0f);
    gl_Position = mvp * vec4(new_pos, 1.0f);
}
