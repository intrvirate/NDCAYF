#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform vec3 tint;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{

    // ambient
    float ambientStrength = 3;
    vec3 ambient = ambientStrength * lightColor;

    //vec3 norm = normalize(Normal);

    vec3 norm = texture(texture_normal1, TexCoords).rgb;
    norm = norm * 2.0 - 1.0;
    norm = normalize(TBN * norm);





    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec4 objectColor = texture(texture_diffuse1, TexCoords);
    vec3 result = (ambient + diffuse) * objectColor.xyz;
    FragColor = vec4(result, objectColor.a);

    if (FragColor.a < 0.1)
    {
        discard;
    }


    //FragColor = texture(texture_diffuse1, TexCoords);
    FragColor.x += tint.x;
    FragColor.y += tint.y;
    FragColor.z += tint.z;



}
