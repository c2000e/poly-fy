#version 430

layout(location = 2) in vec2 tex_coord;
layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D targ_tex;
layout(binding = 1) uniform sampler2D curr_tex;

void main()
{
    color = vec4(abs(texture(targ_tex, tex_coord)
                - texture(curr_tex, tex_coord)).rgb, 1.0);
}
