#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "util/loadMenu.hpp"
//#include "util/object/model.hpp"
#include "util/object/object.h"
#include "util/editor/editor.hpp"
#include "util/bulletDebug/collisiondebugdrawer.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

#include "networkConfig.hpp"
#include "client.hpp"

using namespace std;

uint8_t renderMode = 1; // 1 = fill, 2 = line, 3 = point
GLenum enumRenderMode = GL_FILL;

float cameraSpeed = 0.05f;
float cameraSpeedMultiplier = 4.5f;

float frameTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

double lastX = 400, lastY = 300; //update these to reflect screen size
float yaw = 0;
float pitch = 0;
float sensitivity = 0.25;
float xArrowSensitivity = 0.01;
float yArrowSensitivity = 0.01;

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 cameraRight;
glm::vec3 cameraUp;

glm::vec3 cameraPos = glm::vec3(0,0,0);
glm::vec3 cameraTarget;

bool mouseVisable = false;

glm::vec2 pointMousePos;

string textEntryString = "";
bool inTextBox = false;
bool physicsDebugEnabled = false;
bool showProperties = true;

bool exitMenuVal = false;

void updateCameraFront(double xpos, double ypos) {

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));        //0
    front.y = sin(glm::radians(pitch));                                 //0
    front.z = -cos(glm::radians(yaw)) * cos(glm::radians(pitch));        //-1

    cameraFront = glm::normalize(front);

}

void initializeMouse(GLFWwindow* window){

    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    glfwSetCursorPos(window, width/2, height/2);

    glfwGetCursorPos(window, &lastX, &lastY);
//    printf("initialize cords: x = %f", lastX);
//    printf(" y = %f\n", lastY);
//    printf("initial front vector after:  (x,y,z) (%f,%f,%f)\n", cameraFront.x, cameraFront.y, cameraFront.z);

}

void mouse_callback_camera(GLFWwindow* window, double xpos, double ypos)
{
    updateCameraFront(xpos, ypos);
}

void mouse_callback_point(GLFWwindow* window, double xpos, double ypos)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    pointMousePos.x = -(2*xpos/width)+1;
    pointMousePos.y = -(2*ypos/height)+1;
}

glm::vec2 getMousePos(){
    return pointMousePos;
}

void toggleMouseVisibility(GLFWwindow* window);

void mouse_button_callback_Menu(GLFWwindow* window, int button, int action,
    int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        handleMenuClick();
    }
    if (exitMenuVal == true){
        toggleMouseVisibility(window);
        exitMenuVal = false;
    }
}

void mouse_button_callback_3D(GLFWwindow* window, int button, int action,
    int mods){

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        fprintf(stderr, "hi");

        btVector3 from(0, 20, 0);
        btVector3 to(30, 1, 0);

        //draw debug line
        dynamicsWorld->getDebugDrawer()->drawLine(from, to, btVector4(0, 0, 0, 1));

        btCollisionWorld::AllHitsRayResultCallback allResults(from, to);
        allResults.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
        allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;
        //cast the ray
        dynamicsWorld->rayTest(from, to, allResults);
        btVector3 red(1, 0, 0);
        for (int i = 0; i < allResults.m_hitFractions.size(); i++)
        {
            btVector3 p = from.lerp(to, allResults.m_hitFractions[i]);
            dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, red);
            dynamicsWorld->getDebugDrawer()->drawLine(p, p
                + allResults.m_hitNormalWorld[i], red);
        }
    }


}

void toggleMouseVisibility(GLFWwindow* window){
    initializeMouse(window);
    if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPosCallback(window, mouse_callback_point);
        glfwSetMouseButtonCallback(window, mouse_button_callback_Menu);
        mouseVisable = true;
    }else{
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback_camera);
        glfwSetMouseButtonCallback(window, mouse_button_callback_3D);
        mouseVisable = false;
        inTextBox = false;

    }
}

bool isMouseVisable(){
    return mouseVisable;
}

void exitMenu(){
    exitMenuVal = true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
    int mods)
{
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    else if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        toggleMouseVisibility(window);

    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        showProperties = !showProperties;
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        setCurrentModel();
    }


    if(!mouseVisable){
        // key bondings for camera mode
        if (key == GLFW_KEY_V && action == GLFW_PRESS){

            renderMode++;
            if (renderMode > 3){
                renderMode = 1;
            }
            switch (renderMode) {
            case 1 : enumRenderMode = GL_FILL;  break;
            case 2 : enumRenderMode = GL_LINE;  break;
            case 3 : enumRenderMode = GL_POINT; break;
            }

        }else if(key == GLFW_KEY_C && action == GLFW_PRESS){
            if(!glIsEnabled(GL_CULL_FACE)){
                glEnable(GL_CULL_FACE);
            }else{
                glDisable(GL_CULL_FACE);
            }
        }else if(key == GLFW_KEY_Q && action == GLFW_PRESS){
            physicsDebugEnabled = !physicsDebugEnabled;
        }
    }else{
        if(inTextBox && action != GLFW_RELEASE){
            //key bondings for menu
            if(key >= 32 && key <= 96){
                fprintf(stderr, "%c", (char)key);
                textEntryString += (char)key;
            }else{
                switch (key){
                    case GLFW_KEY_BACKSPACE:
                        if (!textEntryString.empty())
                            textEntryString.pop_back(); //delete last character
                        break;
                    case GLFW_KEY_DELETE:
                        textEntryString = "";
                        break;
                }
            }
            updateMenu();
        }

    }
}

GLenum returnKeysetRenderMode(){
    return enumRenderMode; // points, lines, or triangles
}

glm::vec3 calcCameraMovement(GLFWwindow* window){
    if(!mouseVisable){

        glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraFront));
        glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraUp;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraUp;
        // TODO: Fix the issue where it freaks out and clears the screen
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            updateCameraFront(0, yArrowSensitivity);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            updateCameraFront(0, -yArrowSensitivity);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            updateCameraFront(xArrowSensitivity, 0);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            updateCameraFront(-xArrowSensitivity, 0);
    }
    return cameraPos;
}

float getFrameTime(){
    return frameTime;
}

void calculateFrameTime(){ //call this exactly once per frame
    float currentTime = glfwGetTime();
    frameTime = currentTime - lastFrame;
    lastFrame = currentTime;

    cameraSpeed = cameraSpeedMultiplier * frameTime;
}

glm::vec3 getCameraFront(){
    return cameraFront;
}
