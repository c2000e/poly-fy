#version 430

layout(location = 3) in vec4 in_color;
layout(location = 4) in vec2 tex_coord;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D base_tex;

void main()
{
    out_color = vec4(
            mix(texture(base_tex, tex_coord).rgb, in_color.rgb, in_color.a),
            1.0
    );
}
