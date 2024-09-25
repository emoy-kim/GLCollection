#version 460

layout (location = 0) uniform mat4 ModelViewProjectionMatrix;

layout (location = 0) in vec3 v_position;

out vec3 tex_coord;

void main()
{   
    tex_coord = normalize( v_position );

    gl_Position = ModelViewProjectionMatrix * vec4(v_position, 1.0f);
}