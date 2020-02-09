#ifndef RENDER3D_HPP
#define RENDER3D_HPP

void load3DShaders();
void load3DBuffers();
void load3DMatrices();
void loadAutoMapGen();
void renderLoop3D(GLFWwindow *window);
void cleanup3D();
glm::mat4 getViewMatrix();
glm::mat4 getprojectionMatrix();

#endif // RENDER3D_HPP
