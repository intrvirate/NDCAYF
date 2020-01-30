#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION //stb_image.h wants this here
#include "util/stb_image.h"

#include "util/loadShaders.hpp"
#include "util/handleinput.hpp"
#include "util/otherhandlers.hpp"

#include "util/globalStateHandlers.hpp"

using namespace std;

float vertices2D[] = {
    //positions        //texture cords

    0.80f, 1.00f, 0.0f, 0.00f, 0.00f,
    1.00f, 1.00f, 0.0f, 1.00f, 0.00f,
    1.00f, 0.80f, 0.0f, 1.00f, 1.00f,
    0.80f, 0.80f, 0.0f, 0.00f, 1.00f

};

unsigned indices2D[] = {
    0, 2, 1,
    0, 3, 2
};

//global variables (need to be accessed accross load functions)

GLuint vertexShaderID2D;
GLuint fragmentShaderID2D;
GLuint shaderProgramID2D;

GLuint colorUniform_location;

GLuint VBO2D; //Vertex Buffer Object
GLuint VAO2D; //Vertex Array Object
GLuint EBO2D; //Element Buffer Object

unsigned int textTexture;

//pointer to beginning of text storage array. It's dynamicaly allocated, so just a pointer here.

struct textData * textDataArray;
uint textDataArrayCount = 0;
float textDataSpacing[(8 * 16) - 32];

struct textData {
    string str;
    float size;
    float x;
    float y;
    glm::vec3 activeColor = glm::vec3(0.0f, 0.4f, 0.2f);
    glm::vec3 passiveColor = glm::vec3(0.5f, 0.2f, 0.0f);
};

void loadTextDataSpacing(){

    FILE* my_file = fopen("gamedata/calibri_ofset-12_height-140.csv", "r"); //note:when generating this file, the tool puts other data at the top which must be removed
    int data = 0;
    int actualdata = 0;

    for (int i = 0; i < 32; i++){
        fscanf(my_file, "Char %d Base Width,%d\n", &data, &actualdata);
    }

    for (int i = 0; i < (8 * 16) - 32; i++){
        fscanf(my_file, "Char %d Base Width,%d\n", &data, &actualdata);
        textDataSpacing[i] = actualdata;
    }
}

void set2DletterQuad(char c, float xPos, float yPos, float xSize, float ySize){ //x,y referenced to upper right corner

    c = c-32; //map doesn't contain characters before 32

    int intx = c % 8;
    int inty = c / 8;

    float x = intx;
    float y = inty;

//set vertex cords:
    vertices2D[0] = xPos;
    vertices2D[1] = yPos;

    vertices2D[5] = xPos + xSize;
    vertices2D[6] = yPos;

    vertices2D[10] = xPos + xSize;
    vertices2D[11] = yPos - ySize;

    vertices2D[15] = xPos;
    vertices2D[16] = yPos - ySize;

//set texture cords:
    // 8 = # of colums of characters
    //16 = # of rows of characters
    vertices2D[3] = x/8;
    vertices2D[4] = y/16;

    vertices2D[8] = (x+1)/8;
    vertices2D[9] = y/16;

    vertices2D[13] = (x+1)/8;
    vertices2D[14] = (y+1)/16;

    vertices2D[18] = x/8;
    vertices2D[19] = (y+1)/16;

}

void load2DTextTexture(){

    glGenTextures(1, &textTexture);
    glBindTexture(GL_TEXTURE_2D, textTexture);

    int width, height, nrChannels;
    unsigned char *textTexturedata = stbi_load("gamedata/calibri_ofset-12_height-140.bmp", &width, &height, &nrChannels, 0);
    if(!textTexturedata){
        printf("Failed to load texture");
        exit(0);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textTexturedata);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(textTexturedata);

    //second attribute: texture data
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

}

void load2DShaders(){

    vertexShaderID2D = LoadVertexShader("gamedata/shaders/2Dvertexshader.glsl");
    fragmentShaderID2D = LoadFragmentShader("gamedata/shaders/2Dfragmentshader.glsl");
    shaderProgramID2D = LinkShaders(vertexShaderID2D, fragmentShaderID2D);
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    load2DTextTexture(); //contains second VertexAttribPointer call

    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind VBO
    glBindVertexArray(0); //unbind VAO

    colorUniform_location = glGetUniformLocation(shaderProgramID2D, "textColor");


}

void addTextString(string text, float x, float y, float size, glm::vec3 activeColor, glm::vec3 passiveColor){

    uint oldLength = textDataArrayCount; //this variable is 1 indexed; 0 means array not yet created
    if(textDataArray == NULL){
        //fprintf(stderr, "first time create\n");
        textDataArray = new struct textData [1]; //single value array
        textDataArrayCount++; //should = 1 if being set here
    }else {

        //copy data into new array
        struct textData *newTextDataArray = new struct textData[1 + oldLength]; //make it 1 larger than it already is
        textDataArrayCount++;
        for(uint i = 0; i < oldLength; i++){
            newTextDataArray[i] = textDataArray[i];
        }
        delete[] textDataArray; //clear old array from memory

        textDataArray = newTextDataArray; //assign new array to old mem location
    }
    //add new string to array
    textDataArray[oldLength].str = text;
    textDataArray[oldLength].x = x;
    textDataArray[oldLength].y = y;
    textDataArray[oldLength].size = size;
    textDataArray[oldLength].activeColor = activeColor;
    textDataArray[oldLength].passiveColor = passiveColor;

}

void clearTextStrings(){
    textDataArrayCount = 0;
    delete[] textDataArray;
    textDataArray = NULL;

}

void drawAllText(){ //!!must!! called from within 2D render loop


    if(isMouseVisable() && textDataArray != NULL){ //this code crashes if called on a null array (when no text is in the array)

        for(uint i = 0; i < textDataArrayCount ; i++){ //draw strings
            //fprintf(stderr, "i = %d\n", i);

            float xpos = textDataArray[i].x;

            if(isMouseVisable() && getMousePos().y < textDataArray[i].y && getMousePos().y > textDataArray[i].y - textDataArray[i].size)
                glUniform3f(colorUniform_location, textDataArray[i].activeColor.x, textDataArray[i].activeColor.y, textDataArray[i].activeColor.z);
            else
                glUniform3f(colorUniform_location, textDataArray[i].passiveColor.x, textDataArray[i].passiveColor.y, textDataArray[i].passiveColor.z);

            for(uint j = 0; j < textDataArray[i].str.size(); j++){  //characters
                char c = textDataArray[i].str[j];
                set2DletterQuad(c , xpos  , textDataArray[i].y, textDataArray[i].size, textDataArray[i].size);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2D), vertices2D, GL_STATIC_DRAW);
                glDrawElements(GL_TRIANGLES, sizeof(indices2D) * 3, GL_UNSIGNED_INT, 0);

                xpos += textDataSpacing[(int)c-32] * textDataArray[i].size * 0.0082;
            }
        }
    }
}

void updateFPSCounter(){

    float sec = getFrameTime();
    if(FPScounter){
        sec = 1/sec;
    }

    char str[12]; //12 digits, a few extra to be safe. If this overflows, the pgm crashes
    sprintf(str, "%f", sec);
    glUniform3f(colorUniform_location, 1.0, 1.0, 1.0);//white
    for(int i = 0; i < 7; i++){ //set number of digits to render here
            set2DletterQuad(str[i], -1.0 + (0.025*i), -0.95, 0.05, 0.05);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2D), vertices2D, GL_STATIC_DRAW);
            glDrawElements(GL_TRIANGLES, sizeof(indices2D) * 3, GL_UNSIGNED_INT, 0);
    }
}

void renderLoop2D(GLFWwindow *window){ //called once per frame in the render loop

    glUseProgram(shaderProgramID2D);
    glBindVertexArray(VAO2D);
    glBindTexture(GL_TEXTURE_2D, textTexture);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );

    glBindBuffer(GL_ARRAY_BUFFER, VBO2D);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2D);

    drawAllText(); //don't move this call out of order

    updateFPSCounter();

    glBindVertexArray(0); //unbind VAO

}


