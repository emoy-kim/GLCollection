#version 460

layout (binding = 0) uniform sampler2D Image;

layout (binding = 2, std430) buffer InsideDistanceField { int inside_distance_field[]; };
layout (binding = 3, std430) buffer OutsideDistanceField { int outside_distance_field[]; };

layout (location = 0) out vec4 final_color;

const float zero = 0.0f;
const float one = 1.0f;

void main()
{
    ivec2 pixel_position = ivec2(gl_FragCoord.xy);
    ivec2 layer_size = textureSize( Image, 0 );
    int index = layer_size.x * pixel_position.y + pixel_position.x;

    // for the outside field
    {
        int distance = outside_distance_field[index];
        if (30 < distance && distance <= 50 || 80 < distance && distance <= 100 || 130 < distance && distance <= 150 ||
            180 < distance && distance <= 200 || 230 < distance && distance <= 250 || 280 < distance && distance <= 300 ||
            330 < distance && distance <= 350 || 380 < distance && distance <= 400 || 430 < distance && distance <= 450 ||
            480 < distance && distance <= 500 || 530 < distance && distance <= 550 || 580 < distance && distance <= 600 ||
            630 < distance && distance <= 650 || 680 < distance && distance <= 700 || 730 < distance && distance <= 750 ||
            780 < distance && distance <= 800 || 830 < distance && distance <= 850 || 880 < distance && distance <= 900) {
            float alpha = (one - float(distance) / 2500.0f);
            vec3 outline_color = vec3(one, 0.53f, 0.5f);
            final_color = vec4(outline_color * alpha, one);
            return;
        }
    }

    // for the inside field
    {
        int distance = inside_distance_field[index];
        if (30 < distance && distance <= 50 || 80 < distance && distance <= 100 || 130 < distance && distance <= 150) {
            final_color = vec4(0.32f, 0.77f, 0.83f, one);
            return;
        }
    }

    vec4 color = texture( Image, gl_FragCoord.xy / layer_size );
    if (color.a == zero) discard;

    final_color = color;
}