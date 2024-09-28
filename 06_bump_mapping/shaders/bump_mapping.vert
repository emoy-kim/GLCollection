#version 460

layout (location = 0) uniform mat4 WorldMatrix;
layout (location = 2) uniform mat4 ModelViewProjectionMatrix;

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_tex_coord;

out vec3 position_in_mc;
out vec2 tex_coord;
out vec3 normal_in_mc;
out vec3 tangent_in_mc;
out vec3 binormal_in_mc;

void main()
{   
    position_in_mc = (WorldMatrix * vec4(v_position, 1.0f)).xyz;
    tex_coord = v_tex_coord;
    normal_in_mc = normalize( v_normal );
    tangent_in_mc = vec3(1.0f, 0.0f, 0.0f);
    binormal_in_mc = cross( v_normal, tangent_in_mc );
   
    gl_Position = ModelViewProjectionMatrix * vec4(v_position, 1.0f);
}