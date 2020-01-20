#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "loadShaders.hpp"

GLuint LoadVertexShader(const char * vertexShader_filepath){

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Read Source
    std::string VertexShaderSource;
    std::ifstream VertexShaderStream(vertexShader_filepath, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderSource = sstr.str();
        VertexShaderStream.close();
    }else{
       printf("ERROR: could not open VertexShader source file\n");
       return 0;
    }

    // Compile Source
    char const * VertexSourcePointer = VertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &VertexSourcePointer , NULL);
    glCompileShader(vertexShader);

    // Verify Shader
    GLint successful = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successful);

    if(!successful){
        printf("ERROR: Compiling VertexShader failed\n");
        exit(0);
    }
    if(vertexShader == 0){
        printf("ERROR: Shader ID returned 0\n");
        exit(0);
    }
    return vertexShader;
}

GLuint LoadFragmentShader(const char * fragmentShader_filepath){

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Read Source
    std::string FragmentShaderSource;
    std::ifstream FragmentShaderStream(fragmentShader_filepath, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderSource = sstr.str();
        FragmentShaderStream.close();
    }else{
        printf("ERROR: could not open FragmentShader source file\n");
        return 0;
    }

    // Compile Source
    const char * FragmentSourcePointer = FragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &FragmentSourcePointer, NULL);
    glCompileShader(fragmentShader);

    // Verify Shader
    GLint successful = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successful);

    if(!successful){
        printf("ERROR: Compiling FragmentShader failed\n");
        exit(0);
    }
    if(fragmentShader == 0){
        printf("ERROR: Shader ID returned 0\n");
        exit(0);
    }

    return fragmentShader;
}

GLuint LinkShaders(GLuint vertexShader, GLuint fragmentShader){

    GLuint LinkedProgramID = glCreateProgram();
    glAttachShader(LinkedProgramID, vertexShader);
    glAttachShader(LinkedProgramID, fragmentShader);
    glLinkProgram(LinkedProgramID);

    // Verify Linking
    GLint successful = GL_FALSE;
    glGetProgramiv(LinkedProgramID, GL_LINK_STATUS, &successful);
    if(!successful){
        printf("ERROR: Linking shaders failed\n");
    }

    // Clean up
    glDetachShader(LinkedProgramID, vertexShader);
    glDetachShader(LinkedProgramID, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return LinkedProgramID;
}







