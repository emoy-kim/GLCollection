#version 460

layout (location = 0) uniform mat4 WorldMatrix;
layout (location = 1) uniform mat4 ModelViewProjectionMatrix;
layout (location = 2) uniform float EnvironmentRadius;

layout (location = 0) in vec3 v_position;

out vec2 tex_coord;

void main()
{
    vec4 p = WorldMatrix * vec4(v_position, 1.0f);
    p.xyz /= EnvironmentRadius;
    tex_coord.x = (p.x + 1.0f) * 0.5f;
    tex_coord.y = (p.z + 1.0f) * 0.5f;

    gl_Position = ModelViewProjectionMatrix * vec4(v_position, 1.0f);
}