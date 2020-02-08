#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util/loadShaders.hpp"
#include "util/handleinput.hpp"
#include "util/otherhandlers.hpp"
#include "util/vertexGrid.hpp"
#include "util/groundGridGeneration.hpp"

//global variables (need to be accessed accross load functions)

GLuint vertexShaderID3D;
GLuint fragmentShaderID3D;
GLuint shaderProgramID3D;

GLuint VBO3D; //Vertex Buffer Object
GLuint VAO3D; //Vertex Array Object
GLuint EBO3D; //Element Buffer Object

const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraPos;
glm::vec3 cameraTarget;
glm::vec3 cameraDirection;

glm::mat4 model;
glm::mat4 projection;
glm::mat4 view;

GLuint model_location;
GLuint view_location;
GLuint projection_location;

int incolor;

//map stuff
glm::vec2 rendered_center;
GLuint rendered_center_location;


void load3DShaders(){

    vertexShaderID3D = LoadVertexShader("gamedata/shaders/vertexshader.glsl");
    fragmentShaderID3D = LoadFragmentShader("gamedata/shaders/fragmentshader.glsl");
    shaderProgramID3D = LinkShaders(vertexShaderID3D, fragmentShaderID3D);
    //glUseProgram(shaderProgramID3D);

    //glGetUniformLocation(shaderProgramID3D, "inColor");

}

void load3DBuffers(){
    
    /*
     generate map. if this takes a long time, we might need to move it to 
     when we can display a loading screen
     */
    generateGroundGrid(200, 32); 

    glGenBuffers(1, &VBO3D);
    glGenVertexArrays(1, &VAO3D);
    glGenBuffers(1, &EBO3D);

    glBindVertexArray(VAO3D); //the following stuff is bound to this VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO3D);
    glBufferData(GL_ARRAY_BUFFER, verticesSize_2, vertices_2, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize_2, indices_2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind VBO
    glBindVertexArray(0); //unbind VAO

}

void load3DMatrices(){

    cameraPos = glm::vec3(0.0f, 0.7f, 0.0f);
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraDirection = glm::normalize(cameraPos - cameraTarget);

    view = glm::lookAt(cameraPos, cameraTarget, up);

    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    projection = glm::perspective(glm::radians(45.0f), (float)(1024 / 768), 0.1f, 100.0f); //TODO: update screen size dynamicaly

    //update uniforms
    model_location = glGetUniformLocation(shaderProgramID3D, "model");
    view_location = glGetUniformLocation(shaderProgramID3D, "view");
    projection_location = glGetUniformLocation(shaderProgramID3D, "projection");

    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

}

void loadAutoMapGen(){
    rendered_center_location = glGetUniformLocation(shaderProgramID3D, "rendered_center");
    glUniform2f(rendered_center_location, rendered_center.x, rendered_center.y );
}

void renderLoop3D(GLFWwindow *window){ //called once per frame in the render loop

    //switch to 3D shader
    glUseProgram(shaderProgramID3D);
    glBindVertexArray(VAO3D);
    cameraDirection = getCameraDirection();

    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

    cameraPos = calcCameraMovement(window, cameraPos,  cameraDirection, cameraUp);

    rendered_center.x = (int)cameraPos.x;
    rendered_center.y = (int)cameraPos.z;

    //update matrices
    view = glm::lookAt(cameraPos, cameraPos + cameraDirection, up);
    projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 100.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.5f, 1.0f, 0.0f));

    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform2f(rendered_center_location, rendered_center.x, rendered_center.y );

    //glBindVertexArray(VAO3D);
    glUniform4f(incolor, 1.0f, 0.0f, 0.0f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3D);
    //glDrawElements(GL_TRIANGLES, sizeof(indices) * 3, GL_UNSIGNED_INT, 0);
    glDrawElements(GL_TRIANGLES, indicesSize_2 , GL_UNSIGNED_INT, 0);

    //glBindVertexArray(VAO3D);
    glUniform4f(incolor, 0.0f, 1.0f, 0.0f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, returnKeysetRenderMode());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3D);
    //glDrawElements(GL_TRIANGLES, sizeof(indices) * 3, GL_UNSIGNED_INT, 0);
    glDrawElements(GL_TRIANGLES, indicesSize_2 , GL_UNSIGNED_INT, 0);

}

void cleanup3D(){
    glDeleteVertexArrays(1, &VAO3D);
    glDeleteBuffers(1, &VBO3D);
    glDeleteBuffers(1, &EBO3D);
}

glm::mat4 getViewMatrix(){
    return view;
}

glm::mat4 getprojectionMatrix(){
    return projection;
}
