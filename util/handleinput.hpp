#include <string>
#ifndef HANDLEINPUT_HPP
#define HANDLEINPUT_HPP

extern std::string textEntryString;
extern bool inTextBox;
extern bool physicsDebugEnabled;

extern glm::vec3 cameraFront;
extern glm::vec3 cameraPos;
extern glm::vec3 cameraTarget;
extern glm::vec3 cameraDirection;
extern const glm::vec3 up;

void toggleMouseVisibility(GLFWwindow* window);
void updateCameraFront(double xpos, double ypos);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLenum returnKeysetRenderMode();
glm::vec3 calcCameraMovement(GLFWwindow* window);
float getFrameTime();
glm::vec2 getMousePos();
void calculateFrameTime();
bool isMouseVisable();
void exitMenu();
void initializeMouse(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback_Menu(GLFWwindow* window, int button, int action, int mods);
void mouse_button_callback_3D(GLFWwindow* window, int button, int action, int mods);
glm::vec3 getCameraFront();


#endif // HANDLEINPUT_HPP
