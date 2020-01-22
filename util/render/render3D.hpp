#ifndef RENDER3D_HPP
#define RENDER3D_HPP

void load3DShaders();
void load3DBuffers();
void load3DMatrices();
void loadAutoMapGen();
void renderLoop3D(GLFWwindow *window);
void cleanup3D();

#endif // RENDER3D_HPP
