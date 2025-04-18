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

struct MateralInfo
{
    vec4 EmissionColor;
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
    float SpecularExponent;
};
layout (location = 292) uniform MateralInfo Material;

layout (binding = 0) uniform sampler2D BaseTexture;
layout (binding = 1) uniform sampler2DShadow DepthMap;

layout (location = 1) uniform mat4 ViewMatrix;
layout (location = 297) uniform int UseTexture;
layout (location = 298) uniform int UseLight;
layout (location = 299) uniform int LightIndex;
layout (location = 300) uniform vec4 GlobalAmbient;

in vec3 position_in_ec;
in vec3 normal_in_ec;
in vec2 tex_coord;
in vec4 depth_map_coord;

layout (location = 0) out vec4 final_color;

const float zero = 0.0f;
const float one = 1.0f;
const float half_pi = 1.57079632679489661923132169163975144f;

bool IsPointLight(in vec4 light_position)
{
    return light_position.w != zero;
}

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

float getShadowFactor()
{
    if (zero <= depth_map_coord.x && depth_map_coord.x <= depth_map_coord.w &&
        zero <= depth_map_coord.y && depth_map_coord.y <= depth_map_coord.w &&
        zero <= depth_map_coord.z && depth_map_coord.z <= depth_map_coord.w &&
        zero < depth_map_coord.w) {
        return textureProj( DepthMap, depth_map_coord );
    }
    return one;
}

vec4 calculateLightingEquation()
{
    vec4 color = Material.EmissionColor + GlobalAmbient * Material.AmbientColor;

    if (!bool(Lights[LightIndex].LightSwitch)) return color;

    vec4 light_position_in_ec = ViewMatrix * Lights[LightIndex].Position;

    float final_effect_factor = one;
    vec3 light_vector = light_position_in_ec.xyz - position_in_ec;
    if (IsPointLight( light_position_in_ec )) {
        float attenuation = getAttenuation( light_vector, LightIndex );

        light_vector = normalize( light_vector );
        float spotlight_factor = getSpotlightFactor( light_vector, LightIndex );
        final_effect_factor = attenuation * spotlight_factor;
    }
    else light_vector = normalize( light_position_in_ec.xyz );

    if (final_effect_factor <= zero) return color;

    vec4 local_color = Lights[LightIndex].AmbientColor * Material.AmbientColor;

    float diffuse_intensity = max( dot( normal_in_ec, light_vector ), zero );
    local_color += diffuse_intensity * Lights[LightIndex].DiffuseColor * Material.DiffuseColor;

    vec3 halfway_vector = normalize( light_vector - normalize( position_in_ec ) );
    float specular_intensity = max( dot( normal_in_ec, halfway_vector ), zero );
    local_color +=
        pow( specular_intensity, Material.SpecularExponent ) *
        Lights[LightIndex].SpecularColor * Material.SpecularColor;

    color += local_color * final_effect_factor * getShadowFactor();
    return color;
}

void main()
{
    if (!bool(UseTexture)) final_color = vec4(one);
    else final_color = texture( BaseTexture, tex_coord );

    if (bool(UseLight)) final_color *= calculateLightingEquation();
    else final_color *= Material.DiffuseColor;
}