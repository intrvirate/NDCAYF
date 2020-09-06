#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <btBulletDynamicsCommon.h>

#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"

#include "util/json.hpp"

#include "util/render/render3D.hpp"
#include "util/render/render2D.hpp"

#include "util/loadMenu.hpp"

#include "util/loadShaders.hpp"
#include "util/handleinput.hpp"
#include "util/otherhandlers.hpp"
#include "util/globalStateHandlers.hpp"

#include "util/object/object.h"
#include "util/editor/editor.hpp"

#include "util/bulletDebug/collisiondebugdrawer.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

// this is a horrible way to do this

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include "networking/networkConfig.hpp"
#include "networking/getLan.hpp"
#include "networking/client.hpp"


using json = nlohmann::json;
using namespace std;
using namespace glm;
GLFWwindow* window;

#include <stdint.h>

int main()
{
    // netowrk
    bool networkLoaded = false;
    //=========== SETUP ==========================================================

    //do this first because settings.json will contain things like default window size <-TODO
    setJsonDebugMode(false);
    buildMenu();

    float number;

    uint8_t number2;

    (char)number;

    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    };

    //opengl flags
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //change default window size in otherHandlers.cpp
    window = glfwCreateWindow( CurrentWindowX, CurrentWindowY, "NDCAYF", NULL,
        NULL); //create window. TODO: setting first NULL to glfwGetPrimaryMonitor() results in full screen mode

    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. Double check that the \
            GPU is openGL 3.3 compatible\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    /* TODO: actualy make this work with optimus laptops (it doesn't; you get max fps and pegged gpu)
        //vsync: 1=enabled, 0=disabled, -1=adaptive
        glfwSwapInterval(1);
    */
    glViewport(0, 0, CurrentWindowX, CurrentWindowY);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //allow window to be resized

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    //set up inputs
    glfwSetKeyCallback(window, key_callback);
    glfwPollEvents();

    initializeMouse(window);
    toggleMouseVisibility(window);
    toggleMouseVisibility(window);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // default opengl background on startup: blue

    //loadModels("gamedata/world1.json");
    loadModels("gamedata/scratchpadWorld.json");
    InitializePhysicsWorld();

//=========== IMGUI =========================================================

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    const char* glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_server_window = false;


//=========== RENDER =========================================================

    load3DShaders();
    load3DBuffers();
    load3DMatrices();

    load2DShaders();
    load2DBuffers();
    loadTextDataSpacing();

    loadAutoMapGen();


//================networking stuff====================================
    struct server serverList[MAXSERVERS];

    printf("Loading network\n");
    getAllServers(serverList);

    printServerList(serverList);

    /*
    struct entities all[10];

    // get our id from the server, and the msg
    int clientId;
    struct packet msg;
    if (connectToServer(serverAddr, &clientId, &msg) < 0)
    {
        printf("Failed to connect to: %s\n", inet_ntoa(serverAddr.sin_addr));
    }

    printf("Connection successful to: %s\n", inet_ntoa(serverAddr.sin_addr));
    printf("Data %s  %d   %llu  %s\n", msg.name, msg.ptl, msg.time, msg.extra);
    printf("ID %d\n", clientId);

    setPositions(all, msg.extra);

    cameraPos = glm::vec3(all[clientId].x, all[clientId].y, all[clientId].z);

    printf("%f, %f, %f\n", cameraPos.x, cameraPos.y, cameraPos.z);
    */

//=========== LOOP ===========================================================

    Model *currentModel = getModelPointerByName("Tree03");
    disableCollision(currentModel);
    makeStatic(currentModel);

    Model *lastModel = NULL;    //last pointed-at model
    bool singleScale = true; //ajust scale as single value, or as x, y, and z values


    while( glfwWindowShouldClose(window) == 0){

        //setup stuff that runs regardless of the menu mode
        calculateFrameTime();
        //glClearColor(0.0f, 0.0f, 0.4f, 0.0f); //default background color
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        switch (getLoopMode()){
        case LOOP_MODE_MENU :    {
            renderLoop3D(window);
            renderLoop2D(window);
            drawObjects();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            }
            break;
        case LOOP_MODE_NETWORK : {
            renderLoop3D(window);
            renderLoop2D(window);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();


            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_NoScrollbar;
            window_flags |= ImGuiWindowFlags_NoCollapse;

            ImGui::Begin("Lan View", NULL, window_flags);
            ImGui::Text("All servers:");
            ImGui::Text("");



            for (int j = 0; j < MAXSERVERS; j = j + 1)
            {
                if (strcmp(serverList[j].name, "") != 0)
                {
                    ImGui::Text("Server %s\n", serverList[j].name);
                    //printf("%d  %d\n", servers[j].hasLo, servers[j].loIndex);
                    for (int q = 0; q < serverList[j].numRoutes; q++)
                    {
                        char txt[100];
                        int len = sprintf(txt, "\tIP \"%s\"",
                            (serverList[j].routes[q])) + 5;

                        if (serverList[j].hasLo && q == serverList[j].loIndex)
                        {
                            sprintf(txt, "%s%5s", txt, "LO");
                        }
                        else
                        {
                            sprintf(txt, "%s    ", txt);
                        }

                        if (ImGui::Button(txt))
                        {
                            printf("Server %s, IP %s\n", serverList[j].name,
                                serverList[j].routes[q]);
                        }
                    }
                }
            }

            ImGui::Text("");


            if (ImGui::Button("Update"))
            {
                printf("Loading network\n");

                getAllServers(serverList);
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();


            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
            break;

        case LOOP_MODE_EDIT :    {


            renderLoop3D(window);
            renderLoop2D(window);
            //Bullet Simulation:
            RunStepSimulation();

            debugDraw.SetMatrices(getViewMatrix(), getprojectionMatrix());
            if(physicsDebugEnabled){
                dynamicsWorld->debugDrawWorld();
                //debugDraw.draw();
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();


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
                /*
                currentModel = ((Model *)closestResults.m_collisionObject->getCollisionShape()->getUserPointer());
                //currentModel->tint = glm::vec3(0.2,0.2,0.2);
                currentModel->selected = true;

                if(currentModel != lastModel && lastModel != NULL){
                    //lastModel->tint = glm::vec3(0,0,0);
                    lastModel->selected = false;
                }
                lastModel = currentModel;
                */

                btVector3 p = from.lerp(to,
                    closestResults.m_closestHitFraction);

                dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, blue);
                dynamicsWorld->getDebugDrawer()->drawLine(p, p
                    + closestResults.m_hitNormalWorld, blue);

                updateModelPosition(currentModel, p);

                //ourModel3.setPosition(p);
            }else{
                /*
                if(currentModel != NULL){
                    //currentModel->tint = glm::vec3(0,0,0);
                    currentModel->selected = false;
                }
                */
            }

            //Properties edit window
            drawEditor();

            drawObjects();
            debugDraw.draw();
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            //render imgui (render this last so it's on top of other stuff)
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            }

            break;
        case LOOP_MODE_PLAY :    {
            renderLoop3D(window);
            renderLoop2D(window);
            RunStepSimulation();
            drawObjects();

            debugDraw.SetMatrices(getViewMatrix(), getprojectionMatrix());

            if(physicsDebugEnabled){
                dynamicsWorld->debugDrawWorld();
            }

            debugDraw.draw();

        }
            break;

        case LOOP_MODE_LEGACY :  {
            renderLoop3D(window);
            renderLoop2D(window);

        }
            break;
        }

        // Swap buffers (write to screen)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //cleanup
   // Model::Cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    cleanup3D();
    glfwTerminate();

    return 0;
}
