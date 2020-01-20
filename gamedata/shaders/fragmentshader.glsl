#version 330 core

out vec4 color;
in float outz; //from vertex shader

uniform float inColor;

void main()
{

    //color = inColor;
    color = vec4(0.2, outz, 0.9, 0);
}
