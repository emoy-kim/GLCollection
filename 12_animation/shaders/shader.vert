#version 460

layout (location = 0) uniform mat4 ModelViewProjectionMatrix;

layout (location = 0) in vec3 v_position;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(v_position, 1.0f);
}