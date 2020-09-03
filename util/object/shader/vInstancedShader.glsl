#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

layout (location = 5) in mat4 instanceMatrix;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);

    //gl_Position.x += gl_InstanceID * 2.0;

    Normal = aNormal;

    vec3 T = normalize(vec3(instanceMatrix * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(instanceMatrix * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(instanceMatrix * vec4(aNormal,    0.0)));
    TBN = mat3(T, B, N);

}
