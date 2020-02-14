#ifndef COLLISIONDEBUGDRAWER_HPP
#define COLLISIONDEBUGDRAWER_HPP

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btIDebugDraw.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "util/loadShaders.hpp"

using namespace std;


struct DebugVertex {
    // position
    btVector3 Position;
    // Color
    btVector3 Color;
};

class BulletDebugDrawer_OpenGL : public btIDebugDraw {

public:

    GLuint VBO_debug, VAO_debug;

    vector<DebugVertex> vertices;

    GLuint shaderProgramIDDebug;


    void loadDebugShaders(){
        GLuint vertexShaderIDDebug;
        GLuint fragmentShaderIDDebug;
        vertexShaderIDDebug = LoadVertexShader("gamedata/shaders/vertexshader-BulletDebug.glsl");
        fragmentShaderIDDebug = LoadFragmentShader("gamedata/shaders/fragmentshader-BulletDebug.glsl");
        shaderProgramIDDebug = LinkShaders(vertexShaderIDDebug, fragmentShaderIDDebug);
    }
    void SetMatrices(glm::mat4 pViewMatrix, glm::mat4 pProjectionMatrix)
    {
        glUseProgram(shaderProgramIDDebug);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramIDDebug, "projection"), 1, GL_FALSE, glm::value_ptr(pProjectionMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramIDDebug, "view"), 1, GL_FALSE, glm::value_ptr(pViewMatrix));
    }
    void draw(){ //do the actual rendering here, drawLine just adds lines to the array

        glUseProgram(shaderProgramIDDebug);
        //delete old buffers, then recreate //IS THIS NEEDED???
        glDeleteBuffers(1, &VBO_debug);
        glDeleteVertexArrays(1, &VAO_debug);
        glGenBuffers(1, &VBO_debug);
        glGenVertexArrays(1, &VAO_debug);

        glBindVertexArray(VAO_debug);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_debug);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(DebugVertex), &vertices[0], GL_STREAM_DRAW); //GL_STREAM_DRAW? I think so.

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex),  (void*)offsetof(DebugVertex, Color));

        glBindVertexArray(0);
        glBindVertexArray(VAO_debug); //??????? already bind?
        glDrawArrays(GL_LINES, 0, vertices.size()); //USE?? DrawElements instead?
        glBindVertexArray(0);

        vertices.clear();

    }

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {

        DebugVertex fromV;
        fromV.Position = from;
        fromV.Color = color;

        DebugVertex toV;
        toV.Position = to;
        toV.Color = color;

        vertices.push_back(fromV);
        vertices.push_back(toV);


    }
    virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}
    virtual void reportErrorWarning(const char *) {}
    virtual void draw3dText(const btVector3 &, const char *) {}
    virtual void setDebugMode(int p) {
        m = p;
    }
    int getDebugMode(void) const { return 3; }

    int m;
};

#endif // COLLISIONDEBUGDRAWER_HPP
