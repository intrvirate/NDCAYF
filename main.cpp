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


using json = nlohmann::json;
using namespace std;
using namespace glm;
GLFWwindow* window;

bool test_nw = false;


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

    //loadModels("gamedata/world1.json");
    //loadModels("gamedata/scratchpadWorld.json");
    loadModels("testSaveWorld.json");
    InitializePhysicsWorld();

    load3DShaders();
    load3DBuffers();
    load3DMatrices();
    initializeSkybox();

    load2DShaders();
    load2DBuffers();
    loadTextDataSpacing();

    loadAutoMapGen();

//================networking stuff====================================
    //bool connected = false;
    bool networkLoaded = false;
    setConnection(false);
    struct sockaddr_in serverAddr;
    struct server serverList[MAXSERVERS];

    struct generalPack *dumpPack = new struct generalPack;
    struct generalPack pingPack = makeBasicPack(PING);
    struct generalPack pongPack = makeBasicPack(PONG);

    printf("Loading network\n");
    getAllServers(serverList);

    printServerList(serverList);

    //===entites==
    struct entities players[4];
    unsigned short numEntities = 0;

    int interlopeCount = 0;

    /*

    printf("%f, %f, %f\n", cameraPos.x, cameraPos.y, cameraPos.z);
    */


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
        case LOOP_MODE_NETWORK : {
            if (!isMouseVisable())
            {
                toggleMouseVisibility(window);
            }
            if (connected)
            {
                connected = false;
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
                            printf("Server %s, IP %s\n", serverList[j].name, serverList[j].routes[q]);
                            if (!connectTo(serverList[j].routes[q]))
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

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

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
                int loops = 0;
                dumpPack->protocol = 1000;
                while (waiting)
                {
                    // get msg
                    printf("Waiting\n");
                    printf("asdf%d\n", dumpPack->protocol);
                    loops++;
                    if (checkServer(dumpPack) >= 0)
                    {
                        printf("got one %d\n", dumpPack->protocol);

                        if (dumpPack->protocol == DUMP)
                        {
                            waiting = false;
                            //TODO create/change objects based off of the server data
                            int buf = 0;
                            printf("=====%d, %s, %s, %d, %ld, %ld\n", dumpPack->numObjects, dumpPack->key, dumpPack->name, dumpPack->protocol, dumpPack->time.tv_sec, dumpPack->time.tv_usec);
                            numEntities = dumpPack->numObjects;
                            for (int i = 0; i < dumpPack->numObjects; i++)
                            {
                                if (i == getID())
                                {
                                    printf("us\n");
                                    // get the move data
                                    struct move temp;
                                    memcpy(&temp, &dumpPack->data[buf], sizeof(struct move));
                                    buf += sizeof(struct move);

                                    // player data is set

                                    printf("[%.3f,%.3f,%.3f] before ", cameraPos.x, cameraPos.y, cameraPos.z);
                                    cameraPos = temp.pos;
                                    printf("[%.3f,%.3f,%.3f]\n", cameraPos.x, cameraPos.y, cameraPos.z);
                                    //cameraFront = temp.dir;

                                    // and the last moveID
                                    memcpy(&players[i].moveID, &dumpPack->data[buf], sizeof(unsigned int));
                                    buf += sizeof(unsigned int);

                                }
                                else
                                {
                                    printf("them1\n");
                                    //get the inital int
                                    memcpy(&players[i].numMoves, &dumpPack->data[buf], sizeof(unsigned short));
                                    buf += sizeof(unsigned short);
                                    printf("them2\n");

                                    // get the moves
                                    memcpy(&players[i].moves, &dumpPack->data[buf], sizeof(struct move) * players[i].numMoves);
                                    buf += (sizeof(struct move) * players[i].numMoves);
                                    printf("them3\n");
                                }
                            }


                            //printf("me [%.3f,%.3f,%.3f], server [%.3f,%.3f,%.3f]\n", cameraPos.x, cameraPos.y, cameraPos.z, all[getID()].cameraPos.x, all[getID()].cameraPos.y, all[getID()].cameraPos.z);

                            // TODO force client to be inline with the server
                            // if they were the same at that point and there are points that the server hasn't seen then pretend those are valid
                            // if they are different then move the player based off of the difference between the servers point and the clients equevalent point
                            //reconcileClient(&all[getID()]);

                            //printf("reconcile [%.3f,%.3f,%.3f]\n", all[getID()].cameraPos.x, all[getID()].cameraPos.y, all[getID()].cameraPos.z);
                            interlopeCount = 0;

                        }
                        else
                        {
                            send(pongPack);
                        }
                    }

                }
                printf("Got initial data\n");
                printf("Starting at %.2f, %.2f, %.2f\n", cameraPos.x, cameraPos.y, cameraPos.z);
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
            drawEditor();


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

            if (connected)
            {
                if (checkServer(dumpPack) >= 0)
                {

                    // receve dump and do stuff
                    if (dumpPack->protocol == DUMP)
                    {
                        interlopeCount = 0;
                        //TODO create/change objects based off of the server data

                        int buf = 0;
                        numEntities = dumpPack->numObjects;
                        //printf("dump %lu %lu %d %d\n", dumpPack->numObjects, dumpPack->time.tv_sec, dumpPack->time.tv_usec, dumpPack->protocol);
                        printf("=====%s, %s, %d, %ld, %ld\n", dumpPack->key, dumpPack->name, dumpPack->protocol, dumpPack->time.tv_sec, dumpPack->time.tv_usec);
                        dumpPack->protocol = (unsigned short)1000;
                        for (int i = 0; i < dumpPack->numObjects; i++)
                        {
                            if (i == getID())
                            {
                                // get the move data
                                struct move temp;
                                memcpy(&temp, &dumpPack->data[buf], sizeof(struct move));
                                buf += sizeof(struct move);

                                cameraPos = temp.pos;
                                printf("us %.2f, %.2f, %.2f\n", temp.pos.x, temp.pos.y, temp.pos.z);
                                //cameraFront = temp.dir;

                                // and the last moveID
                                memcpy(&players[i].moveID, &dumpPack->data[buf], sizeof(unsigned int));
                                buf += sizeof(unsigned int);

                            }
                            else
                            {
                                printf("them\n");
                                //get the inital int
                                memcpy(&players[i].numMoves, &dumpPack->data[buf], sizeof(unsigned short));
                                buf += sizeof(unsigned short);

                                // get the moves
                                memcpy(&players[i].moves, &dumpPack->data[buf], sizeof(struct move) * players[i].numMoves);
                                buf += (sizeof(struct move) * players[i].numMoves);
                            }
                        }

                        // TODO force client to be inline with the server
                        // if they were the same at that point and there are points that the server hasn't seen then pretend those are valid
                        // if they are different then move the player based off of the difference between the servers point and the clients equevalent point
                        //reconcileClient(&all[getID()]);
                        }
                    }

                    //interlope
                    string pink = "PINK";
                    string blue = "BLUE";
                    string green = "GREN";
                    string orange = "ORNG";
                    string names[4] = {blue, pink, green, orange};


                    int name = 0;
                    for (int i = 0; i < numEntities; i++)
                    {
                        if (i != getID())
                        {
                            // applies the next move
                            if (interlopeCount < players[i].numMoves)
                            {
                                Model *temp = getModelPointerByName(names[name]);
                                //printf("updating %s's pos\n", names[name].c_str());
                                updateModelPosition(temp, players[i].moves[interlopeCount].pos);
                                //printf("\tbefore [%.3f,%.3f,%.3f]\n", all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                                //applyKeys(all[i].keys[interlopeCount].moves, all[i].keys[interlopeCount].dir, &(all[i].cameraPos));
                                //printf("\tafter [%.3f,%.3f,%.3f]\n", all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                                //btVector3 infront(all[i].cameraPos.x, all[i].cameraPos.y, all[i].cameraPos.z);
                                //ourModel5.setPosition(infront);
                            }
                        }
                        name++;
                    }
                    // next time we will do the next one
                    interlopeCount++;
                }
                //draw the players

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
    saveJson("testSaveWorld.json");

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

