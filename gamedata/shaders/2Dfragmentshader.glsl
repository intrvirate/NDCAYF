#version 330 core

in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture;

void main()
{
    color = texture(ourTexture, TexCoord);
    if(color.x < 0.5){ //delete for transperent background
        discard;
    }

    //color = vec4(color.x, color.y, 0.5, 1.0);
}







































