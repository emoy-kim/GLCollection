#version 460

layout (location = 0) uniform mat4 ModelViewProjectionMatrix;

layout (location = 0) in vec3 v_position;
layout (location = 2) in vec2 v_tex_coord;

out vec2 tex_coord;

void main()
{
    tex_coord = v_tex_coord;

    gl_Position = ModelViewProjectionMatrix * vec4(v_position, 1.0f);
}