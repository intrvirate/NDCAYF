#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

//change these to change the default size of the window
int CurrentWindowX = 2048;
int CurrentWindowY = 768;
float CurrentWindowRatio = (float)CurrentWindowX/CurrentWindowY ;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){

    CurrentWindowX = width;
    CurrentWindowY = height;
    CurrentWindowRatio = (float)CurrentWindowX/CurrentWindowY ;

    glViewport(0, 0, width, height);
}
