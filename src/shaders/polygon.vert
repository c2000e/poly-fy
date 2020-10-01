#version 430

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 position;
layout(location = 2) out vec4 out_color;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    out_color = in_color;
}
