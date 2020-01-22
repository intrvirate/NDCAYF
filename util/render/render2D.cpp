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


float vertices2D[] = {
    1.00f, 1.00f, 0.0f,
    0.90f, 1.00f, 0.0f,
    1.00f, 0.90f, 0.0f,
    0.90f, 0.90f, 0.0f
};

unsigned indices2D[] = {
    0, 1, 2,
    1, 3, 2
};


//global variables (need to be accessed accross load functions)

GLuint vertexShaderID2D;
GLuint fragmentShaderID2D;
GLuint shaderProgramID2D;

GLuint VBO2D; //Vertex Buffer Object
GLuint VAO2D; //Vertex Array Object
GLuint EBO2D; //Element Buffer Object


void load2DShaders(){

    vertexShaderID2D = LoadVertexShader("gamedata/shaders/2Dvertexshader.glsl");
    fragmentShaderID2D = LoadFragmentShader("gamedata/shaders/2Dfragmentshader.glsl");
    shaderProgramID2D = LinkShaders(vertexShaderID2D, fragmentShaderID2D);
    //glUseProgram(shaderProgramID2D);

}

void load2DBuffers(){
    glGenBuffers(1, &VBO2D);
    glGenVertexArrays(1, &VAO2D);
    glGenBuffers(1, &EBO2D);

    glBindVertexArray(VAO2D); //the following stuff is bound to this VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO2D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2D), vertices2D, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2D), indices2D, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind VBO
    glBindVertexArray(0); //unbind VAO

}

void renderLoop2D(GLFWwindow *window){ //called once per frame in the render loop

    glUseProgram(shaderProgramID2D);
    glBindVertexArray(VAO2D);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2D);
    glDrawElements(GL_TRIANGLES, sizeof(indices2D) * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0); //unbind VAO

}
