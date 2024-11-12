#version 460

layout (location = 1) uniform vec3 Color;

layout (location = 0) out vec4 final_color;

void main()
{
    final_color = vec4(Color, 1.0f);
}