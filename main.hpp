#ifndef MAIN_HPP
#define MAIN_HPP

extern GLFWwindow* window;

void enterMenu();
void enterNetwork();
void enterEdit();
void enterPlay();
void enterLegacy();
void leaveMenu();
void leaveNetwork();
void leaveEdit();
void leavePlay();
void leaveLegacy();
void runTransitionFunctions();
void glfw_error_callback(int error, const char* description);


#endif // MAIN_HPP
