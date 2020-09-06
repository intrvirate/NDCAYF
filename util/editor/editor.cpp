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

Model *currentModel = NULL;

void setCurrentModel()
{
    if (currentModel == NULL)
    {
        btVector3 from(cameraPos.x,cameraPos.y,cameraPos.z);
        btVector3 to(cameraPos.x+cameraFront.x*100,
        cameraPos.y+cameraFront.y*100, cameraPos.z+cameraFront.z*100);

        btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
        closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
        closestResults.m_collisionFilterGroup = COL_SELECTER;
        closestResults.m_collisionFilterMask = COL_SELECT_RAY_COLLIDES_WITH;

        dynamicsWorld->rayTest(from, to, closestResults);

        if (closestResults.hasHit() && !isMouseVisable())
        {
            currentModel = ((Model *)closestResults.m_collisionObject->\
                getCollisionShape()->getUserPointer());
        disableCollision(currentModel);
        makeStatic(currentModel);
        }

    } else
    {
        enableCollision(currentModel);
        makeDynamic(currentModel);
        currentModel = NULL;

    }
    // currentModel = getModelPointerByName("Tree03");
}

void drawEditor()
{
    btVector3 from(cameraPos.x,cameraPos.y,cameraPos.z);
    btVector3 to(cameraPos.x+cameraFront.x*100,
    cameraPos.y+cameraFront.y*100, cameraPos.z+cameraFront.z*100);

    btVector3 blue(0.1, 0.3, 0.9);

    dynamicsWorld->getDebugDrawer()->drawSphere(btVector3(0,0,0),
        0.5, blue); //at origin
    btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
    closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
    closestResults.m_collisionFilterGroup = COL_SELECTER;
    closestResults.m_collisionFilterMask = COL_SELECT_RAY_COLLIDES_WITH;

    dynamicsWorld->rayTest(from, to, closestResults);

    if (closestResults.hasHit() && !isMouseVisable())
    {

        btVector3 p = from.lerp(to,
            closestResults.m_closestHitFraction);

        dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, blue);
        dynamicsWorld->getDebugDrawer()->drawLine(p, p
            + closestResults.m_hitNormalWorld, blue);

        if (currentModel != NULL)
        {
            updateModelPosition(currentModel, p);
        }
    }

    if(showProperties)
    {


        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_NoResize;

        ImGui::Begin("Properties", NULL, window_flags);
        ImGui::Text("Hello world!");

        ImGui::End();
    }

}


