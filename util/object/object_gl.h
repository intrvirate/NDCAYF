#ifndef OBJECT_GL_H
#define OBJECT_GL_H
#include "object.h"
/*
 * Whenever any of the functions in object.cpp need to
 * do any opengl stuff, it is done via functions in here.
 * this allows object.cpp to be reused in the server
 * codebase without modification by using a object_gl
 * file that doesn't do anything.
 *
 * Doing this because keeping 2 versions of object.cpp
 * in sync would be a nightmare.
 *
 */

void loadShader_gl(int i);
void unloadModels_buffers_gl(int i, int j);
void unloadModels_shaders_gl(int i);
void processMesh_buffers_gl(Mesh *mesh);
unsigned int TextureFromFile_gl(string filename);
float getPhysicsFrameTime();
void drawObjects();

#endif // OBJECT_GL_H
