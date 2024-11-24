#version 460

#define MAX_LIGHTS 32

struct LightInfo
{
    int LightSwitch;
    vec4 Position;
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
    vec3 SpotlightDirection;
    float SpotlightCutoffAngle;
    float SpotlightFeather;
    float FallOffRadius;
};
layout (location = 4) uniform LightInfo Lights[MAX_LIGHTS];

struct MateralInfo {
    vec4 EmissionColor;
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
    float SpecularExponent;
};
layout (location = 292) uniform MateralInfo Material;

layout (binding = 0) uniform sampler2D BaseTexture;

layout (location = 1) uniform mat4 ViewMatrix;
layout (location = 297) uniform int UseLight;
layout (location = 298) uniform int LightIndex;
layout (location = 299) uniform float EnvironmentRadius;
layout (location = 300) uniform vec4 GlobalAmbient;

in vec3 position_in_wc;
in vec3 normal_in_wc;
in vec3 eye_position_in_wc;

in vec3 light_position_in_ec;
in vec3 position_in_ec;
in vec3 normal_in_ec;

in vec2 tex_coord;

layout (location = 0) out vec4 final_color;

const float zero = 0.0f;
const float one = 1.0f;
const float pi = 3.14159265358979323846264338327950288f;
const float half_pi = 1.57079632679489661923132169163975144f;

float getAttenuation(in vec3 light_vector, in int light_index)
{
    float squared_distance = dot( light_vector, light_vector );
    float distance = sqrt( squared_distance );
    float radius = Lights[light_index].FallOffRadius;
    if (distance <= radius) return one;

    return clamp( radius * radius / squared_distance, zero, one );
}

float getSpotlightFactor(in vec3 normalized_light_vector, in int light_index)
{
    if (Lights[light_index].SpotlightCutoffAngle >= 180.0f) return one;

    vec4 direction_in_ec = transpose( inverse( ViewMatrix ) ) * vec4(Lights[light_index].SpotlightDirection, zero);
    vec3 normalized_direction = normalize( direction_in_ec.xyz );
    float factor = dot( -normalized_light_vector, normalized_direction );
    float cutoff_angle = radians( clamp( Lights[light_index].SpotlightCutoffAngle, zero, 90.0f ) );
    if (factor >= cos( cutoff_angle )) {
        float normalized_angle = acos( factor ) * half_pi / cutoff_angle;
        float threshold = half_pi * (one - Lights[light_index].SpotlightFeather);
        return normalized_angle <= threshold ?
            one :
            cos( half_pi * (normalized_angle - threshold) / (half_pi - threshold) );
    }
    return zero;
}

vec4 calculateLightingEquation()
{
    vec4 color = Material.EmissionColor + GlobalAmbient * Material.AmbientColor;
    if (!bool(Lights[LightIndex].LightSwitch)) return color;

    float final_effect_factor = one;
    vec3 light_vector = light_position_in_ec - position_in_ec;
    float attenuation = getAttenuation( light_vector, LightIndex );

    light_vector = normalize( light_vector );
    float spotlight_factor = getSpotlightFactor( light_vector, LightIndex );
    final_effect_factor = attenuation * spotlight_factor;
    if (final_effect_factor <= zero) return color;

    vec4 local_color = Lights[LightIndex].AmbientColor * Material.AmbientColor;

    float diffuse_intensity = max( dot( normal_in_ec, light_vector ), zero );
    local_color += diffuse_intensity * Lights[LightIndex].DiffuseColor * Material.DiffuseColor;

    vec3 halfway_vector = normalize( light_vector - normalize( position_in_ec ) );
    float specular_intensity = max( dot( normal_in_ec, halfway_vector ), zero );
    local_color +=
        pow( specular_intensity, Material.SpecularExponent ) *
        Lights[LightIndex].SpecularColor * Material.SpecularColor;

    color += local_color * final_effect_factor;
    return color;
}

/*
   Hemisphere Equation: x^2 + y^2 + z^2 = 1, 0 <= y <= 1

   Reflected View Vector: (x-xw)/rx = (y-yw)/ry = (z-zw)/rz
    -> object point x in wc = (xw, yw, zw)
    -> normalized reflected r = (rx, ry, rz)

   Intersection Point: (rx * t + xw, ry * t + yw, rz * t + zw), some real t >= 0

   Solve Equation: (rx * t + xw)^2 + (ry * t + yw)^2 + (rz * t + zw)^2 = 1
    -> t1 = -(r dot x) + sqrt((r dot x)^2 - (x^2 - 1))
    -> t2 = -(r dot x) - sqrt((r dot x)^2 - (x^2 - 1))
*/
vec3 calculateReflectedTextureInWC()
{
    vec3 view_vector = normalize( position_in_wc - eye_position_in_wc );
    vec3 normal = normalize( normal_in_wc );
    vec3 reflected = normalize( reflect( view_vector, normal ) );

    vec3 reflected_color = vec3(zero);
    if (dot( reflected, normal ) >= zero) {
        float r_dot_x = dot( reflected, position_in_wc );
        float d =
            sqrt( r_dot_x * r_dot_x - (dot( position_in_wc, position_in_wc ) - EnvironmentRadius * EnvironmentRadius) );
        float t1 = -r_dot_x + d;
        float t2 = -r_dot_x - d;
        vec3 point1 = t1 * reflected + position_in_wc;
        vec3 point2 = t2 * reflected + position_in_wc;
        vec3 intersection_point;
        if (t1 >= zero && point1.y >= zero) intersection_point = point1 / EnvironmentRadius;
        else if (t2 >= zero && point2.y >= zero) intersection_point = point2 / EnvironmentRadius;
        else return reflected_color;

        vec2 texture_point;
        texture_point.x = (intersection_point.x + 1.0f) * 0.5f;
        texture_point.y = (intersection_point.z + 1.0f) * 0.5f;
        reflected_color = texture( BaseTexture, texture_point ).rgb;
    }
    return reflected_color;
}

void main()
{
    final_color = texture( BaseTexture, tex_coord );

    if (bool(UseLight)) final_color *= calculateLightingEquation();
    else final_color *= Material.DiffuseColor;

    final_color.xyz += calculateReflectedTextureInWC();
}