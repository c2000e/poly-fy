#version 430

layout(location = 2) in vec2 tex_coord;
layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    color = texture(tex, tex_coord);
}
