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

Model *pickedModel = NULL;
Model *cursoredModel = NULL;
btVector3 p;
string modelName = "";
string cursoredModelName = "";
bool modelDynamic;
bool cursoredModelDynamic;
float yOffset = 0;
float yRotateOffset = 0;
float yIncrement = 0.125;
float yRotateIncrement = 0.125;

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
        printf("rotate\n");
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
    // pickedModel = getModelPointerByName("Tree03");
}

void drawEditor()
{

    draw3dCursor();


    if(showProperties)
    {

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_NoResize;
        ImVec2 windowSize;

        if (pickedModel != NULL || cursoredModel != NULL)
        {
            windowSize = ImVec2(ImGui::GetFontSize() * 20.0f, 90);
        } else
        {
            windowSize = ImVec2(ImGui::GetFontSize() * 20.0f, 0);
        }
        ImGui::SetNextWindowSize(windowSize);
        ImVec2 windowPos = ImVec2(25.0f, 25.0f);
        ImGui::SetNextWindowPos(windowPos);

        string scrollModeText;
        switch (scrollMode)
        {
            case 1 : scrollModeText = "Translate"; break;
            case 2 : scrollModeText = "Rotate"; break;
        }
        ImGui::Begin("Properties", NULL, window_flags);
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20.0f);

        if (pickedModel != NULL)
        {
            ImGui::Text("Picked:");
            ImGui::SameLine();
            ImGui::Text(modelName.c_str());
            ImGui::Text(scrollModeText.c_str());

            if (modelDynamic)
            {
                ImGui::Text("Is Dynamic");
            } else
            {
                ImGui::Text("Is Static");
            }
        } else if (cursoredModel != NULL)
        {
            ImGui::Text("Cursored:");
            ImGui::SameLine();
            ImGui::Text(cursoredModelName.c_str());
            ImGui::Text(scrollModeText.c_str());

            if (cursoredModelDynamic)
            {
                ImGui::Text("Is Dynamic");
            } else
            {
                ImGui::Text("Is Static");
            }
        }
        ImGui::End();

    }

}


