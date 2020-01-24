#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main(){

    gl_Position = vec4(aPos.x , aPos.y, aPos.z , 1.0);
    TexCoord = aTexCoord;

}
