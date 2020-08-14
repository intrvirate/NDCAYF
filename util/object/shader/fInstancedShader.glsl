#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{

    FragColor = texture(texture_diffuse1, TexCoords);

    if (FragColor.a < 0.3)
    {
        discard;
    }




}
