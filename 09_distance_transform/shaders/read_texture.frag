#version 460

layout (binding = 0) uniform sampler2D BaseTexture;

in vec2 tex_coord;

layout (location = 0) out vec4 final_color;

void main()
{
    final_color = texture( BaseTexture, tex_coord );
    if (final_color.a == 0.0f) discard;
}