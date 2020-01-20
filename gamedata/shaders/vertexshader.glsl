#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out float outz;

//map generation variables:
uniform vec2 rendered_center; //cordinates of the center of the currently rendered portion of the map

void main(){

    //gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); //2D
    //swap y and z so that z is up in world cordinates (as far as the model is concerned)
    float z = cos(aPos.y + rendered_center.y ) + cos(aPos.y + rendered_center.y )*sin(aPos.x + rendered_center.x);
    gl_Position = projection * view * model * vec4(aPos.x + rendered_center.x , cos(aPos.y + rendered_center.y ) + cos(aPos.y + rendered_center.y )*sin(aPos.x + rendered_center.x) , aPos.y  + rendered_center.y , 1.0);
    gl_PointSize = 4;
    outz = (z+2)/4;
}
