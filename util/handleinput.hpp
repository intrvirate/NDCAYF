#ifndef HANDLEINPUT_HPP
#define HANDLEINPUT_HPP

void toggleMouseVisibility(GLFWwindow* window);
void updateCameraFront(double xpos, double ypos);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLenum returnKeysetRenderMode();
glm::vec3 calcCameraMovement(GLFWwindow* window, glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp);
float getFrameTime();
glm::vec3 getMousePos();
void calculateFrameTime();
bool isMouseVisable();
void initializeMouse(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
glm::vec3 getCameraDirection();


#endif // HANDLEINPUT_HPP
