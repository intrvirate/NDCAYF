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
#include "util/handleinput.hpp"

#include "util/browser/Browser.hpp"

Browser* save_browser = NULL;
Browser* open_browser = NULL;

Model *pickedModel = NULL;
Model *cursoredModel = NULL;
btVector3 p;
string modelName = "";
string cursoredModelName = "";
bool modelDynamic;
bool cursoredModelDynamic;

float yOffset = 0;
float yIncrement = 0.125;

float yRotateOffset = 0;
float yRotateIncrement = 0.261799387799;

float scaleIncrement = 0.01;

bool needSave = false;
bool needOpen = false;

//clears pickedModel and cursoredModel.
//ueed when the engine switches worlds,
//causing these pointers to become invalid.
void clearEditorPointers(){
    pickedModel = NULL;
    cursoredModel = NULL;
    needSave = false;
    needOpen = false;
    modelName = "";
    cursoredModelName = "";
}

void editorTranslateY(int direction)
{
    if (pickedModel != NULL)
    {
        if (direction > 0)
        {
            yOffset += yIncrement;
        } else if (direction < 0)
        {
            yOffset -= yIncrement;
        }

    }
}

void editorRotateY(int direction)
{
    if (pickedModel != NULL)
    {
        if (direction > 0)
        {
            updateRelativeModelRotation(pickedModel,
                glm::vec3(0, -yRotateIncrement, 0));
        } else if (direction < 0)
        {
            updateRelativeModelRotation(pickedModel,
                glm::vec3(0, yRotateIncrement, 0));
        }

    }
}

void editorScale(int direction)
{
    if (pickedModel != NULL)
    {
        if (direction > 0)
        {
            updateRelativeScale(pickedModel, glm::vec3(scaleIncrement,
                scaleIncrement, scaleIncrement));
        } else if (direction < 0)
        {
            updateRelativeScale(pickedModel, glm::vec3(0 - scaleIncrement,
                0 - scaleIncrement, 0 - scaleIncrement));
        }
    }
}

void draw3dCursor()
{
    btVector3 from(cameraPos.x,cameraPos.y,cameraPos.z);
    btVector3 to(cameraPos.x+cameraFront.x*100,
    cameraPos.y+cameraFront.y*100, cameraPos.z+cameraFront.z*100);

    btVector3 blue(0.1, 0.3, 0.9);
    //at origin
    dynamicsWorld->getDebugDrawer()->drawSphere(btVector3(0,0,0), 0.5, blue);
    btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
    closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
    closestResults.m_collisionFilterGroup = COL_SELECTER;
    closestResults.m_collisionFilterMask = COL_SELECT_RAY_COLLIDES_WITH;

    dynamicsWorld->rayTest(from, to, closestResults);

    if (closestResults.hasHit() && !isMouseVisable())
    {
        cursoredModel = ((Model *)closestResults.m_collisionObject->\
            getCollisionShape()->getUserPointer());

        cursoredModelName = cursoredModel->objectPath;
        cursoredModelDynamic = cursoredModel->isDynamic;
        size_t delimPos = cursoredModelName.find_last_of("/");
        cursoredModelName = cursoredModelName.substr(delimPos + 1);

        p = from.lerp(to,
            closestResults.m_closestHitFraction);

        dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, blue);
        dynamicsWorld->getDebugDrawer()->drawLine(p, p
            + closestResults.m_hitNormalWorld, blue);

    } else
    {
        cursoredModel = NULL;
        cursoredModelDynamic = false;
    }

    if (pickedModel != NULL && cursoredModel != NULL)
    {
        p.setY(p.getY() + (btScalar)yOffset);
        updateModelPosition(pickedModel, p);
    } else if (cursoredModel != NULL)
    {
        yOffset = 0.0f;
    }
}

void setPickedModel()
{
    if (cursoredModel != NULL)
    {

        if (pickedModel == NULL)
        {

            pickedModel = cursoredModel;

            modelDynamic = pickedModel->isDynamic;

            disableCollision(pickedModel);
            makeStatic(pickedModel);

            modelName = pickedModel->objectPath;
            size_t delimPos = modelName.find_last_of("/");
            modelName = modelName.substr(delimPos + 1);

        } else
        {
            enableCollision(pickedModel);
            makeDynamic(pickedModel);
            modelName = "";
            pickedModel = NULL;
        }
    }
}

void drawEditor()
{

    draw3dCursor();

    if (needSave)
    {
        if (save_browser == NULL)
        {
            save_browser = new Browser();
        }
        save_browser->draw();
        if (save_browser->hasSelected())
        {
            needSave = false;
            std::cout << "savepath: " << save_browser->getSelection() << std::endl;
            delete save_browser;
            save_browser = NULL;
        }
    }

    if (needOpen)
    {
        if (open_browser == NULL)
        {
            open_browser = new Browser();
        }
        open_browser->draw();

        if (open_browser->hasSelected())
        {
            needOpen = false;
            std::cout << "openPath: " << save_browser->getSelection() << std::endl;
            delete open_browser;
            open_browser = NULL;
        }
    }


    if (showProperties)
    {

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_NoResize;
        ImVec2 windowSize;

        if (pickedModel != NULL || cursoredModel != NULL)
        {
            windowSize = ImVec2(ImGui::GetFontSize() * 15.0f, 90);
        } else
        {
            windowSize = ImVec2(ImGui::GetFontSize() * 15.0f, 0);
        }
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(ImVec2(25.0f, 25.0f));

        string scrollModeText;
        switch (scrollMode) {
            case 1 : scrollModeText = "Translate"; break;
            case 2 : scrollModeText = "Rotate"; break;
            case 3 : scrollModeText = "Scale"; break;
        }

        ImGui::Begin("Properties", NULL, window_flags);
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20.0f);

        if (pickedModel != NULL) {
            ImGui::Text("Picked:");
            ImGui::SameLine();
            ImGui::Text("%s",modelName.c_str());
            ImGui::Text("%s",scrollModeText.c_str());

            if (modelDynamic)
            {
                ImGui::Text("Is Dynamic");
            } else
            {
                ImGui::Text("Is Static");
            }
        }
        else if (cursoredModel != NULL) {
            ImGui::Text("obj:");
            ImGui::SameLine();
            ImGui::Text("%s",cursoredModelName.c_str());
            ImGui::Text("%s",scrollModeText.c_str());

            if (cursoredModelDynamic)
            {
                ImGui::Text("Is Dynamic");
            } else
            {
                ImGui::Text("Is Static");
            }
        }
        else {
            //show save/load buttons only when escaped
            // FIXME: This doesn't work, it shows open/save when pointed at skybox
            if (ImGui::Button("save world")) {
                needSave = true;
            }
            if (ImGui::Button("open world")) {
                needOpen = true;
            }
        }
        ImGui::End();

    }

}


