#version 330 core

in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture;

void main()
{
    color = texture(ourTexture, TexCoord);
    //color = vec4(0.8, 0.3, 0.3, 1.0);
}







































