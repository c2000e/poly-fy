#version 430

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 in_tex_coord;
layout(location = 2) out vec2 out_tex_coord;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    out_tex_coord = in_tex_coord;
}
