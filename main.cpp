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

#include "util/object/mesh.hpp"
#include "util/object/shader.hpp"
#include "util/object/model.hpp"

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


using json = nlohmann::json;
using namespace std;
using namespace glm;
GLFWwindow* window;

int main()
{
    // netowrk
    bool networkLoaded = false;
    //=========== SETUP ==========================================================

    //do this first because settings.json will contain things like default window size
    setJsonDebugMode(false);
    buildMenu();

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
    window = glfwCreateWindow( CurrentWindowX, CurrentWindowY, "NDCAYF", NULL, NULL); //create window. TODO: setting first NULL to glfwGetPrimaryMonitor() results in full screen mode
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. Double check that the GPU is openGL 3.3 compatible\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    /* TODO: actualy make this work with optimus laptops (it doesn't; you get mag fps and pegged gpu)
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

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // default opengl background on startup: blue


    //TODO: move this out of main; this is just for testing
    Shader ourShader("util/object/shader/vShader.glsl", "util/object/shader/fShader.glsl");


    Model ourModel("obj/objects/terrain03.obj", false, new btSphereShape(btScalar(1.)), 0.0, btVector3(0,-8,0),btVector3(100,100,100));

    Model ourModel2("obj/objects/plannets/moon.obj", true, new btSphereShape(btScalar(5.)), 1.0 , btVector3(0,50,0),btVector3(5,5,5));

    Model ourModel3("obj/objects/building02.obj", false, new btSphereShape(btScalar(1.)), 0.0 , btVector3(1,-30,0),btVector3(1,1,1));

    Model::InitializeModelPhysicsWorld();

//=========== IMGUI =========================================================


IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO(); (void)io;
ImGui::StyleColorsDark();
//ImGui::StyleColorsClassic();


//bindings: consider writing our own. this is the default demo one provided
const char* glsl_version = "#version 130";
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init(glsl_version);

bool show_demo_window = true; //TODO why is this on a different tab level?
    bool show_another_window = false;
    bool show_server_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


//=========== RENDER =========================================================

    load3DShaders();
    load3DBuffers();
    load3DMatrices();

    load2DShaders();
    load2DBuffers();
    loadTextDataSpacing();

    loadAutoMapGen();

//=========== LOOP ===========================================================


    Model *currentModel = NULL; //current pointed-at model
    Model *lastModel = NULL;    //last pointed-at model

    while( glfwWindowShouldClose(window) == 0){

        //setup
        calculateFrameTime();
        //glClearColor(0.0f, 0.0f, 0.4f, 0.0f); //default background color
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderLoop3D(window);
        renderLoop2D(window);

        //Bullet Simulation:
        Model::RunStepSimulation();

        //draw debug line


        //draw debug stuff from bullet
        debugDraw.SetMatrices(getViewMatrix(), getprojectionMatrix());

        if(physicsDebugEnabled){
            dynamicsWorld->debugDrawWorld();
            //debugDraw.draw();
        }

    //=-----=-=-=-=-==--=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-^-^-^-^-^-^-^-

        btVector3 from(cameraPos.x,cameraPos.y,cameraPos.z);
        btVector3 to(cameraPos.x+cameraFront.x*100,cameraPos.y+cameraFront.y*100,cameraPos.z+cameraFront.z*100);

        btVector3 blue(0.1, 0.3, 0.9);
        dynamicsWorld->getDebugDrawer()->drawSphere(btVector3(0,0,0), 0.5, blue); //at origin

        btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
        closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

        dynamicsWorld->rayTest(from, to, closestResults);

        if (closestResults.hasHit())
        {
            currentModel = ((Model *)closestResults.m_collisionObject->getCollisionShape()->getUserPointer());
            currentModel->tint = glm::vec3(0.2,0.2,0.2);

            if(currentModel != lastModel && lastModel != NULL){
                lastModel->tint = glm::vec3(0,0,0);
            }
            lastModel = currentModel;

            btVector3 p = from.lerp(to, closestResults.m_closestHitFraction);
            dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, blue);
            dynamicsWorld->getDebugDrawer()->drawLine(p, p + closestResults.m_hitNormalWorld, blue);

            //ourModel3.setPosition(p);
        }else{
            currentModel->tint = glm::vec3(0,0,0);
        }

        debugDraw.draw();

//end physics loop=====================================================================================

    //============imgui===========
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::Checkbox("Demo Window", &show_demo_window);
        ImGui::Checkbox("Another Window", &show_another_window);
        ImGui::Checkbox("Server Information", &show_server_window);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        if (show_another_window){
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        if (show_server_window)
        {
            ImGui::Begin("Server Information", &show_server_window);
            ImGui::Text("Assorted Server Information");

            // ================= trying to implement networking/client.c =======
            struct server serverList[MAXSERVERS];


            if (!networkLoaded)
            {
                printf("yeehaw");
                getAllServers(serverList);
                networkLoaded = true;
            }

            //for (int j = 0; j < MAXSERVERS; j = j + 1)
            //{
            //    if (strcmp(serverList[j].name, "") != 0)
            //    {
            //        printf("For server %s\n", serverList[j].name);
            //        //printf("%d  %d\n", servers[j].hasLo, servers[j].loIndex);
            //        for (int q = 0; q < serverList[j].numRoutes; q++)
            //        {
            //            printf("\tFound route \"%s\"", inet_ntoa(serverList[j].routes[q].sin_addr));
            //            if (serverList[j].hasLo && q == serverList[j].loIndex)
            //            {
            //                printf("\tLO");
            //            }
            //            printf("\n");
            //        }
            //    }
            //}

            if (ImGui::Button("exit"))
            {
                show_server_window = false;
            }
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    //end imgui

        ourShader.use();

        ourModel.Draw(ourShader);
        ourModel2.Draw(ourShader);
        ourModel3.Draw(ourShader);


        //render imgui (render this last so it's on top of other stuff
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //draw model physics debug
    Model::DrawDebugModels();


    //cleanup physics code
    //TODO: clean this up

    Model::Cleanup();

    cleanup3D();
    glfwTerminate();

    return 0;
}
