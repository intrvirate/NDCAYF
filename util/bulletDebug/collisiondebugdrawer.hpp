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


class BulletDebugDrawer_OpenGL : public btIDebugDraw {
    GLuint VBO_debug, VAO_debug;

    GLuint shaderProgramIDDebug;
public:
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

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        //fprintf(stderr, "from x = %f", from.x());
        //fprintf(stderr, " y = %f", from.y());
        // fprintf(stderr, " z = %f", from.z());

        //fprintf(stderr, " to x = %f", to.x());
        //  fprintf(stderr, " y = %f", to.y());
        //   fprintf(stderr, " z = %f\n", to.z());

        // Vertex data
        GLfloat points[12];

        points[0] = from.x();
        points[1] = from.y();
        points[2] = from.z();
        points[3] = color.x();
        points[4] = color.y();
        points[5] = color.z();

        points[6] = to.x();
        points[7] = to.y();
        points[8] = to.z();
        points[9] = color.x();
        points[10] = color.y();
        points[11] = color.z();

        glUseProgram(shaderProgramIDDebug);

        glDeleteBuffers(1, &VBO_debug);
        glDeleteVertexArrays(1, &VAO_debug);

        glGenBuffers(1, &VBO_debug);
        glGenVertexArrays(1, &VAO_debug);

        glBindVertexArray(VAO_debug);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_debug);

        glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); // &points vs points??

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

        glBindVertexArray(0);
        glBindVertexArray(VAO_debug);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);

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
