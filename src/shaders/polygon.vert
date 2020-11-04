#version 430

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 position;
layout(location = 2) in vec2 origin;
layout(location = 3) out vec4 out_color;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_ClipDistance[4];
};

const float SCREEN_SIZE = 512.0;
const float CELL_SIZE = SCREEN_SIZE / 4;

void main()
{
    gl_Position = vec4((position - 0.5 * SCREEN_SIZE) / (0.5 * SCREEN_SIZE),
            0.0, 1.0);

    vec2 bottom_left = origin;
    vec2 top_right = origin + vec2(CELL_SIZE, CELL_SIZE);

    // Clip vertices that aren't within the correct cell.
    gl_ClipDistance[0] = position.x - bottom_left.x;
    gl_ClipDistance[1] = top_right.x - position.x;
    gl_ClipDistance[2] = position.y - bottom_left.y;
    gl_ClipDistance[3] = top_right.y - position.y;

    out_color = in_color;
}
