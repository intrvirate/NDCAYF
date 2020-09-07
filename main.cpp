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

//#include "util/object/mesh.hpp"
//#include "util/object/shader.hpp"
//#include "util/object/model.hpp"

#include "util/object/object.h"

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
#include <sys/time.h>

#include "util/networking/networkConfig.hpp"
#include "util/networking/getLan.hpp"
#include "util/networking/client.hpp"


using json = nlohmann::json;
using namespace std;
using namespace glm;
GLFWwindow* window;

bool test_nw = true;

int main()
{
    setTestNw(test_nw);
    if (test_nw)
    {
        struct entities alls[10];

        /*
        glm::vec3 newPoss = glm::vec3(-1.694,9.585,-14.178);
        glm::vec3 fakeDirs = glm::vec3(-0.839,0.358,0.409);


                        printf("before [%.3f,%.3f,%.3f]\n", newPoss.x, newPoss.y, newPoss.z);
            applyKeys("wd", fakeDirs, &newPoss);

                        printf("after [%.3f,%.3f,%.3f]\n", newPoss.x, newPoss.y, newPoss.z);
                        */


        int x = sizeof(struct generalPack);
        int y = 10;
        int z = 10;
        int a = sizeof(unsigned short int);
        int b = sizeof(struct timeval);
        int c = 1000;

        struct timeval tv;

        gettimeofday(&tv, NULL);

        /*
        printf("%d ", y);
        printf("%d ", z);
        printf("%d ", a);
        printf("%d ", b);
        printf("%d\n", c);
        printf("Theoritical %d\n", y+z+a+b+c);
        printf("Actual      %d\n", x);

        printf("%ld, %ld\n", tv.tv_sec, tv.tv_usec);
        */


        // to send
        struct generalPack pack;

        // to recieve
        struct generalPack *msgPack = new struct generalPack;

        // add to pack
        strcpy(pack.key, "key");
        strcpy(pack.name, "key");
        pack.protocol = PING;
        pack.time = tv;
        //strcpy(pack.data, "1234567891");

        struct sockaddr_in serverAddra;
        struct server serverLista[MAXSERVERS];

        printf("Loading network\n");
        getAllServers(serverLista);

        printServerList(serverLista);


        // set the server
        struct sockaddr_in serverAddr;

        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        //setServerAddr(serverAddr);
        char ip[] = "10.55.20.48";
        if (!connectTo(ip))
        {
            printf("Failed to connect\n");
        }

        /*
        printf("made a socket\n");
        if (makeSocket() < 0)
        {
            printf("failed to make\n");
        }
        */


        long sum = 0;
        int numberOfPings = 1000;
        for (int i = 0; i < numberOfPings; i++)
        {
            gettimeofday(&pack.time, NULL);
            send(pack);
            //printf("\t%s, %s, %d, %ld, %ld\n", pack.key, pack.name, pack.protocol, pack.time.tv_sec, pack.time.tv_usec);


            if (checkServer(msgPack) < 0)
            {
                printf("failed to receve\n");
            }
            gettimeofday(&(msgPack->time), NULL);

            //printf("\t%s, %s, %d, %ld, %ld\n", msgPack->key, msgPack->name, msgPack->protocol, msgPack->time.tv_sec, msgPack->time.tv_usec);

            long diff = msgPack->time.tv_usec - pack.time.tv_usec;
            sum += diff;
            //printf("lag in microseconds %ld\n", diff);
        }
        printf("Average %ld, total %ld\n", sum / numberOfPings, sum);

        glm::vec3 Pos = glm::vec3(1.0f,1.0f,1.0f);
        printf("%.2f %.2f %.2f\n", Pos.x, Pos.y, Pos.z);

        glm::vec3 cameraFront(0,0,0);
        char keys[5] = "hi .";
        netLog(Pos, cameraFront, keys);




        return 0;
        /*
        struct MsgPacket pack;
        int numAlls = 0;
        processMsg(tmsg10, &pack);

        printf("name %s, ptl %d, time %llu, data %s\n\n", pack.name, pack.ptl, pack.time, pack.data);

        applyDumpData(alls, pack.data, &numAlls);

        for (int i = 0; i < numAlls; i++)
        {
            if (i != getID())
            {
                printf("Num of moves %d\n", alls[i].numMoves);
            }
        }

        /*
        reconcileClient(alls[getID()], cameraPos);
{
    glm::vec3 cameraPos;
    glm::vec3 cameraDirection;
    struct move moves[100];
    int numMoves;
};
         */

        /*
        printf("asdfasfd%d\n", numAlls);
        for (int i = 0; i < numAlls; i++)
        {
            printf("Pos [%.3f,%.3f,%.3f], Dir [%.3f,%.3f,%.3f]\n", alls[i].cameraPos.x, alls[i].cameraPos.y, alls[i].cameraPos.z,
                alls[i].cameraDirection.x, alls[i].cameraDirection.y, alls[i].cameraDirection.z);
            if (i == 0)
            {
                printf("this is this comupter\n");

            }
            else
            {
                for (int j = 0; j < alls[i].numMoves; j++)
                {
                    printf("\tMove, dir [%.3f,%.3f,%.3f], keys [%s]\n", alls[i].keys[j].dir.x, alls[i].keys[j].dir.y, alls[i].keys[j].dir.z,
                        alls[i].keys[j].moves);
                }
            }
        }
        */


        return -1;
    }

    // netowrk
    bool networkLoaded = false;
    //=========== SETUP ==========================================================

    //do this first because settings.json will contain things like default window size <-TODO
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

    loadModels("gamedata/world1.json");
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
    //bool connected = false;
    setConnection(false);
    struct sockaddr_in serverAddr;
    struct server serverList[MAXSERVERS];

    printf("Loading network\n");
    getAllServers(serverList);

    printServerList(serverList);

    //===entites==
    struct entities all[10];
    int numEntities = 0;

    int interlopeCount = 0;


    /*

    printf("%f, %f, %f\n", cameraPos.x, cameraPos.y, cameraPos.z);
    */

//=========== LOOP ===========================================================

    //Model *currentModel = &ourModel; //current pointed-at model
    Model *lastModel = NULL;    //last pointed-at model
    bool showProperties = true;
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
            if (!isMouseVisable())
            {
                toggleMouseVisibility(window);
            }
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
                        int len = sprintf(txt, "\tIP \"%s\"", (serverList[j].routes[q])) + 5;

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
                            printf("Server %s, IP %s\n", serverList[j].name, serverList[j].routes[q]);
                            if (connectTo(serverList[j].routes[q]) < 0)
                            {
                                printf("Failed to connect to: %s at %s\n", serverList[j].name, serverList[j].routes[q]);
                            }
                            else
                            {
                                connected = true;
                            }
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

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();


            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (connected)
            {
                serverAddr = getServerAddr();
                printf("Connection successful to: %s\n", inet_ntoa(serverAddr.sin_addr));
                //printf("Data %s  %d   %llu  %s\n", msg.name, msg.ptl, msg.time, msg.data);
                printf("ID %d\n", getID());
                printf("===================Waiting for server=================\n");
                setLoopMode(LOOP_MODE_EDIT);

                //setPositions(all, msg.data);
                // wait for the server to send the info
                bool waiting = true;
                /*
                while (waiting)
                {
                    char buf[BUFSIZE*2];
                    //struct MsgPacket msg;
                    strcpy(buf, "");
                    int type;
                    // get msg
                    if (checkServer(buf) > 0)
                    {
                        type = processMsg(buf, &msg);

                        if (type == DUMP)
                        {
                            waiting = false;
                            applyDumpData(all, msg.data, &numEntities);

                            //printf("me [%.3f,%.3f,%.3f], server [%.3f,%.3f,%.3f]\n", cameraPos.x, cameraPos.y, cameraPos.z, all[getID()].cameraPos.x, all[getID()].cameraPos.y, all[getID()].cameraPos.z);

                            reconcileClient(&all[getID()]);

                            //printf("reconcile [%.3f,%.3f,%.3f]\n", all[getID()].cameraPos.x, all[getID()].cameraPos.y, all[getID()].cameraPos.z);

                            interlopeCount = 0;

                            for (int i = 0; i < numEntities; i++)
                            {
                                if (i != getID())
                                {
                                    btVector3 infront(all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                                    //ourModel5.setPosition(infront);
                                    //printf("Num of moves %d\n", all[i].numMoves);
                                }
                                else
                                {
                                    cameraPos = all[i].cameraPos;
                                    cameraFront = all[i].cameraDirection;
                                }
                            }

                        }
                    }

                }
                */

                cameraPos = all[getID()].cameraPos;
                cameraFront = all[getID()].cameraDirection;
            }



            }
            break;
        case LOOP_MODE_EDIT :    {


            renderLoop3D(window);
            renderLoop2D(window);
            //Bullet Simulation:
            RunStepSimulation();
            drawObjects();
            debugDraw.SetMatrices(getViewMatrix(), getprojectionMatrix());
            if(physicsDebugEnabled){
                dynamicsWorld->debugDrawWorld();
                //debugDraw.draw();
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();


            btVector3 from(cameraPos.x,cameraPos.y,cameraPos.z);
            btVector3 to(cameraPos.x+cameraFront.x*100,cameraPos.y+cameraFront.y*100,cameraPos.z+cameraFront.z*100);

            btVector3 blue(0.1, 0.3, 0.9);

            dynamicsWorld->getDebugDrawer()->drawSphere(btVector3(0,0,0), 0.5, blue); //at origin
            btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
            closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

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
                btVector3 p = from.lerp(to, closestResults.m_closestHitFraction);
                dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, blue);
                dynamicsWorld->getDebugDrawer()->drawLine(p, p + closestResults.m_hitNormalWorld, blue);

                //ourModel3.setPosition(p);
            }else{
                /*
                if(currentModel != NULL){
                    //currentModel->tint = glm::vec3(0,0,0);
                    currentModel->selected = false;
                }
                */
            }
/*
            if(showProperties){ //Properties edit window
                ImGuiWindowFlags window_flags = 0;
                window_flags |= ImGuiWindowFlags_NoScrollbar;
                window_flags |= ImGuiWindowFlags_NoResize;
                window_flags |= ImGuiWindowFlags_NoCollapse;

                ImGui::Begin("Properties", NULL, window_flags);
                ImGui::Text(currentModel->objectPath.c_str()); //name of object file
                ImGui::Checkbox("single scale value", &singleScale);
                //scaling change

                if(singleScale){

                    ImGui::SliderFloat("scale", &(currentModel->scale[0]), 0.1f, 100.0f, "%1.0f");
                    currentModel->scale[1] = currentModel->scale[0];
                    currentModel->scale[2] = currentModel->scale[0];

                    currentModel->syncScale();


                }else{
                    ImGui::InputFloat("scale X", &(currentModel->scale[0]), 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("scale Y", &(currentModel->scale[1]), 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("scale Z", &(currentModel->scale[2]), 0.01f, 1.0f, "%.3f");
                    currentModel->syncScale();
                }

                //position change
                btVector3 pos = currentModel->body->getWorldTransform().getOrigin();
                ImGui::SliderFloat("pos X", &(pos[0]), -100.0f, 100.0f, "%10.0f");
                ImGui::SliderFloat("pos Y", &(pos[2]), -100.0f, 100.0f, "%10.0f");
                ImGui::SliderFloat("pos Z", &(pos[1]), -100.0f, 100.0f, "%10.0f");
                currentModel->body->getWorldTransform().setOrigin(pos);

                ImGui::End();

            }
*/
            debugDraw.draw();
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            //render imgui (render this last so it's on top of other stuff)
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            //=== do the network

            if (connected)
            {
                char buf[BUFSIZE*2];
                //struct MsgPacket msg;
                strcpy(buf, "");
                int type;
                // get msg
                /*
                if (checkServer(buf) > 0)
                {
                    type = processMsg(buf, &msg);

                    if (type == DUMP)
                    {
                        applyDumpData(all, msg.data, &numEntities);

                        printf("me [%.3f,%.3f,%.3f], server [%.3f,%.3f,%.3f]\n",
                            cameraPos.x, cameraPos.y, cameraPos.z,
                            all[getID()].cameraPos.x, all[getID()].cameraPos.y, all[getID()].cameraPos.z);

                        reconcileClient(&all[getID()]);

                        printf("reconcile [%.3f,%.3f,%.3f]\n",
                            all[getID()].cameraPos.x, all[getID()].cameraPos.y, all[getID()].cameraPos.z);

                        interlopeCount = 0;

                        for (int i = 0; i < numEntities; i++)
                        {
                            if (i != getID())
                            {
                                btVector3 infront(all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                                //ourModel5.setPosition(infront);
                                //printf("Num of moves %d\n", all[i].numMoves);
                            }
                            else
                            {
                                cameraPos = all[i].cameraPos;
                            }
                        }

                    }
                }
                */
                    //interlope

                for (int i = 0; i < numEntities; i++)
                {
                    if (i != getID())
                    {
                        //printf("Num of moves %d\n", all[i].numMoves);
                        // applies the next move
                        if (interlopeCount < all[i].numMoves)
                        {
                            //printf("\tbefore [%.3f,%.3f,%.3f]\n", all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                            //applyKeys(all[i].keys[interlopeCount].moves, all[i].keys[interlopeCount].dir, &(all[i].cameraPos));
                            //printf("\tafter [%.3f,%.3f,%.3f]\n", all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                            btVector3 infront(all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                            //ourModel5.setPosition(infront);
                        }
                    }
                }
                // next time we will do the next one
                interlopeCount++;
            }
                //draw the players
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
