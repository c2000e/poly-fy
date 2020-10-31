#version 430

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 position;
layout(location = 2) in vec2 cell_center;
layout(location = 3) out vec4 out_color;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_ClipDistance[1];
};

const float CELL_SIZE = 2.0 / 4;

float box_sdf(vec2 p, vec2 b)
{
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    gl_ClipDistance[0] = -box_sdf(position - cell_center, vec2(0.5 * CELL_SIZE,
                0.5 * CELL_SIZE));

    out_color = in_color;
}
