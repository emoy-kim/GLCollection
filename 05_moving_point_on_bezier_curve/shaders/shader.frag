#version 460

layout (location = 1) uniform vec4 Color;

layout (location = 0) out vec4 final_color;

void main()
{
    final_color = Color;
}