#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{

    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vec4 world_pos = (model * vec4(aPos, 1.0)) + vec4((normalize(aNormal)*0.002*gl_Position.z).xyz, 0.0);
    gl_Position = projection * view * world_pos;

}
