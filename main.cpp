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
#include <thread>
#include <future>

#include <btBulletDynamicsCommon.h>

#include "main.hpp"

#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"

#include "util/json.hpp"

#include "util/render/render3D.hpp"
#include "util/render/render2D.hpp"
#include "util/render/skybox.hpp"

#include "util/loadMenu.hpp"

#include "util/loadShaders.hpp"
#include "util/handleinput.hpp"
#include "util/otherhandlers.hpp"
#include "util/globalStateHandlers.hpp"

#include "util/object/object.h"
#include "util/object/object_gl.h"
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
#include <sys/time.h>
#include <stdint.h>

#include "util/networking/networkConfig.hpp"
#include "util/networking/getLan.hpp"
#include "util/networking/client.hpp"

#include "util/networking/TCP/TCP.hpp"
#include "util/networking/TCP/MusicStreamer.hpp"
#include "util/networking/TCP/FileGet.hpp"
#include "util/networking/TCP/FileUpload.hpp"


#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

using json = nlohmann::json;
using namespace std;
using namespace glm;
GLFWwindow* window;

bool test_nw = true;


int main()
{
    if (test_nw)
    {
        //makeTCP();
        //tcpConnect("10.55.20.48");
        //tcpMain("10.55.22.125");
        string dir("songs/");
        string filename("bee.wav");

        /*
        struct server serverList[MAXSERVERS];

        if (makeSocket() < 0)
        {
            perror("Failed to get socket");
        }

        printf("Loading network\n");
        getAllServers(serverList);
        */

        string song = "NoWay.wav";

        //ofstream theFile("songs/bee.wav");
        //ofstream theFile("songs/bee.wav");

        Music fobj("10.55.76.19");
        FileUpload fobj1("10.55.76.19", dir, filename, MAP);
        FileGet fobj2("10.55.76.19");
        //fobj.run();
        thread athread(&TCP::run, fobj);

        athread.join();

        exit(-1);

        return 0;
    }

    //=========== SETUP ==========================================================

    //do this first because settings.json will contain things like default window size <-TODO
    setJsonDebugMode(false);
    buildMenu();

    float number;

    uint8_t number2;

    (char)number;

    glfwSetErrorCallback(glfw_error_callback);
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW.");
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
    //TODO This is for testing purposes, we'll implement it properly in a bit.
    glfwSetScrollCallback(window, editorScrollCallback);
    glfwPollEvents();

    initializeMouse(window);
    toggleMouseVisibility(window);
    toggleMouseVisibility(window);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

//=========== IMGUI =========================================================

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    const char* glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

//=========== RENDER =========================================================

    loadModels("gamedata/world1.json");
    //loadModels("gamedata/scratchpadWorld.json");
    //loadModels("testSaveWorld.json");

    load3DShaders();
    load3DBuffers();
    load3DMatrices();
    initializeSkybox();

    load2DShaders();
    load2DBuffers();
    loadTextDataSpacing();

    loadAutoMapGen();

//================networking stuff====================================
    struct sockaddr_in serverAddr;
    struct server serverList[MAXSERVERS];

    struct generalPack *dumpPack = new struct generalPack;
    struct generalPack pingPack = makeBasicPack(PING);
    struct generalPack pongPack = makeBasicPack(PONG);
    struct generalPack infoPack = makeBasicPack(INFO);

    if (makeSocket() < 0)
    {
        perror("Failed to get socket");
    }

    printf("Loading network\n");
    getAllServers(serverList);

    printServerList(serverList);

    //===entites==
    struct entities players[4];
    unsigned short numEntities = 0;

    int interlopeCount = 0;


//=========== LOOP ===========================================================


    //Model *testingModel = getModelPointerByName("Tree");
    //updateModelRotation(testingModel, glm::quat(1,1,1,1));
    //float inc = 1;
    //uint8_t tick = 0;
    glClearColor(0.2f, 0.2f, 0.3f, 0.0f); // default opengl background on startup: blue

    while( glfwWindowShouldClose(window) == 0){

        //setup stuff that runs regardless of the menu mode
        calculateFrameTime();
        //glClearColor(0.0f, 0.0f, 0.4f, 0.0f); //default background color
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        runTransitionFunctions();

        switch (getLoopMode()){
        case LOOP_MODE_MENU :    {
            renderLoop3D(window);
            renderLoop2D(window);
            drawObjects();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            renderSkybox();

            }
            break;
        case LOOP_MODE_NETWORK :
        {
            if (!isMouseVisable())
            {
                toggleMouseVisibility(window);
            }
            // we were connected but arn't now so set to false
            // interlope needs to be set at the begining
            if (getConnection())
            {
                setConnection(false);
                interlopeCount = 0;
            }
            renderLoop3D(window);
            renderLoop2D(window);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();

            makeServerListWindow(serverList);

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (getConnection())
            {
                serverAddr = getServerAddr();
                printf("Connection successful to: %s as id: %d\n", inet_ntoa(serverAddr.sin_addr), getID());
                printf("===================Waiting for server=================\n");

                // wait for the server to send the info
                // loops keeps it from consuming the client
                bool waiting = true;
                int loops = 0;
                while (waiting)
                {
                    // only try for so long
                    loops++;
                    if (loops > 500)
                    {
                        // failed to get data so we arn't connected anymore
                        waiting = false;
                        setConnection(false);
                    }
                    if (checkServer(dumpPack) >= 0)
                    {
                        //printf("got one %d\n", dumpPack->protocol);

                        if (dumpPack->protocol == DUMP)
                        {
                            waiting = false;

                            int buf = 0;

                            printf("Got the dump=====%d, %s, %s, %d, %ld, %ld\n", dumpPack->numObjects,
                                dumpPack->key, dumpPack->name, dumpPack->protocol,
                                dumpPack->time.tv_sec, dumpPack->time.tv_usec);

                            // loads the data into the right spot, all dumps only have players rn
                            numEntities = dumpPack->numObjects;
                            for (int i = 0; i < dumpPack->numObjects; i++)
                            {
                                if (i == getID())
                                {
                                    // get the move data
                                    struct move temp;
                                    memcpy(&temp, &dumpPack->data[buf], sizeof(struct move));
                                    buf += sizeof(struct move);

                                    // player data is set
                                    cameraPos = temp.pos;

                                    // move number is synced
                                    memcpy(&players[i].moveID, &dumpPack->data[buf], sizeof(unsigned int));
                                    buf += sizeof(unsigned int);
                                    setMove((players[i].moveID + (unsigned int)1));

                                }
                                else
                                {
                                    //get the number of moves int
                                    memcpy(&players[i].numMoves, &dumpPack->data[buf], sizeof(unsigned short));
                                    buf += sizeof(unsigned short);

                                    // use to get all the moves out
                                    memcpy(&players[i].moves, &dumpPack->data[buf], sizeof(struct move) * players[i].numMoves);
                                    buf += (sizeof(struct move) * players[i].numMoves);
                                }
                            }
                        }
                        else
                        {
                            send(&pongPack);
                        }
                    }
                }
                // we shall exit to the play loop, also make mouse go away
                if (getConnection())
                {
                    printf("Starting at %.2f, %.2f, %.2f\n", cameraPos.x, cameraPos.y, cameraPos.z);
                    setLoopMode(LOOP_MODE_PLAY);
                    if (isMouseVisable())
                    {
                        toggleMouseVisibility(window);
                    }
                }
            }
        }
        break;
        case LOOP_MODE_EDIT :    {

            renderLoop3D(window);
            renderLoop2D(window);
            //Bullet Simulation:
            RunStepSimulation();


            //tick++;
            //if (tick > 50){
                //inc = 0.01f;
                //updateRelativeModelRotation(testingModel, glm::vec3(inc,0,0));
            //}

            debugDraw.SetMatrices(getViewMatrix(), getprojectionMatrix());
            if(physicsDebugEnabled){
                dynamicsWorld->debugDrawWorld();
                //debugDraw.draw();
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //Properties edit window
fprintf(stderr, "got to drawEditor");
            drawEditor();
fprintf(stderr, "got past drawEditor");
            drawObjects();
            debugDraw.draw();
            renderSkybox();
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

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


            //=== do the network
            if (getConnection())
            {
                if (checkServer(dumpPack) >= 0)
                {
                    if (dumpPack->protocol == DUMP)
                    {
                        // reset the count to we start interloping at the begining
                        interlopeCount = 0;

                        printf("=====%s, %s, %d, %ld, %ld\n", dumpPack->key, dumpPack->name,
                            dumpPack->protocol, dumpPack->time.tv_sec, dumpPack->time.tv_usec);

                        // keep our place
                        int buf = 0;
                        numEntities = dumpPack->numObjects;
                        for (int i = 0; i < dumpPack->numObjects; i++)
                        {
                            if (i == getID())
                            {
                                // get the move data
                                struct move temp;
                                memcpy(&temp, &dumpPack->data[buf], sizeof(struct move));
                                buf += sizeof(struct move);

                                // get the id of this move
                                memcpy(&players[i].moveID, &dumpPack->data[buf], sizeof(unsigned int));
                                buf += sizeof(unsigned int);

                                /*
                                if (!(temp.pos.x == cameraPos.x && temp.pos.y == cameraPos.y && temp.pos.z == cameraPos.z))
                                {
                                    //debug
                                    printf("server %.2f, %.2f, %.2f == ", temp.pos.x, temp.pos.y, temp.pos.z);
                                    printf("us %.2f, %.2f, %.2f\n", cameraPos.x, cameraPos.y, cameraPos.z);
                                }
                                */

                                // only change cameraPos if the server and client don't agree on where the player was at the servers id
                                reconcileClient(&players[i], &temp, &cameraPos);
                            }
                            else
                            {
                                //printf("them11\n");
                                //get the inital int
                                memcpy(&players[i].numMoves, &dumpPack->data[buf], sizeof(unsigned short));
                                buf += sizeof(unsigned short);
                                //printf("them12\n");

                                // get the moves
                                // had abug here
                                memcpy(&players[i].moves, &dumpPack->data[buf], sizeof(struct move) * players[i].numMoves);
                                buf += (sizeof(struct move) * players[i].numMoves);
                                //printf("them13\n");
                            }
                        }
                    }
                }

                // temporary stuff
                string pink = "PINK";
                string blue = "BLUE";
                string green = "GREN";
                string orange = "ORNG";
                string names[4] = {blue, pink, green, orange};


                // goes through each entity and applies its interlope point
                // increments the interlopeCount by one afterwards so we get the next one
                for (int i = 0; i < numEntities; i++)
                {
                    if (i != getID())
                    {
                        if (interlopeCount < players[i].numMoves)
                        {
                            Model *temp = getModelPointerByName(names[i]);
                            updateModelPosition(temp, players[i].moves[interlopeCount].pos);
                        }
                    }
                }
                interlopeCount++;
            }

            debugDraw.draw();
            renderSkybox();

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


//mode transition functions: these are called
//once when the mode changes to be the listed mode
void enterMenu(){
    fprintf(stderr, "entered menu\n");

}
void enterNetwork(){
    fprintf(stderr, "entered network\n");

}
void enterEdit(){
    fprintf(stderr, "entered edit\n");

}
void enterPlay(){
    fprintf(stderr, "entered play\n");

}
void enterLegacy(){
    fprintf(stderr, "entered legacy\n");
    //saveJson("testSaveWorld.json");
    unloadModels();
}

void leaveMenu(){
    fprintf(stderr, "left menu\n");

}
void leaveNetwork(){
    fprintf(stderr, "left network\n");

}
void leaveEdit(){
    fprintf(stderr, "left edit\n");

}
void leavePlay(){
    fprintf(stderr, "left play\n");

}
void leaveLegacy(){
    fprintf(stderr, "left legacy\n");
    loadModels("gamedata/scratchpadWorld.json");


}

void runTransitionFunctions(){
    if(getLoopMode() != getOldLoopMode()){

        switch (getOldLoopMode()){
            case LOOP_MODE_MENU:{
                leaveMenu();
                break;
            }
            case LOOP_MODE_NETWORK: {
                leaveNetwork();
                break;
            }
            case LOOP_MODE_EDIT: {
                leaveEdit();
                break;
            }
            case LOOP_MODE_PLAY:{
                leavePlay();
                break;
            }
            case LOOP_MODE_LEGACY:{
                leaveLegacy();
                break;
            }
        }
        setOldLoopMode(getLoopMode());
        switch (getLoopMode()){
            case LOOP_MODE_MENU:{
                enterMenu();
                break;
            }
            case LOOP_MODE_NETWORK: {
                enterNetwork();
                break;
            }
            case LOOP_MODE_EDIT: {
                enterEdit();
                break;
            }
            case LOOP_MODE_PLAY:{
                enterPlay();
                break;
            }
            case LOOP_MODE_LEGACY:{
                enterLegacy();
                break;
            }
        }
    }
}

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
}
