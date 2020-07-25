#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 tint;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);
    FragColor.x += tint.x;
    FragColor.y += tint.y;
    FragColor.z += tint.z;

}
