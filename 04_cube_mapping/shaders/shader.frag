#version 460

layout (binding = 0) uniform samplerCube BaseTexture;

layout (location = 1) uniform vec4 Color;

layout (location = 0) out vec4 final_color;

in vec3 tex_coord;

void main()
{
    final_color = texture( BaseTexture, tex_coord ) * Color;
}