#ifndef EDITOR_H
#define EDITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <iostream>
#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"

#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

#include "util/json.hpp"

#include "util/object/object.h"

void editorTranslateY(int direction);
void editorRotateY(int direction);
void editorScale(int direction);
void draw3dCursor();
void drawEditor();
extern void setPickedModel();
#endif
