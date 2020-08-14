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

#include "util/object/mesh.hpp"
#include "util/object/shader.hpp"
#include "util/object/model.hpp"

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

#include "networking/networkConfig.hpp"
#include "networking/getLan.hpp"
#include "networking/client.hpp"


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
    toggleMouseVisibility(window);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // default opengl background on startup: blue


    //TODO: move this out of main; this is just for testing
    Shader ourShader("util/object/shader/vShader.glsl", "util/object/shader/fShader.glsl");
    Shader outlineShader("util/object/shader/VoutlineShader.glsl", "util/object/shader/FoutlineShader.glsl");
    Shader instancedShader("util/object/shader/vInstancedShader.glsl", "util/object/shader/fInstancedShader.glsl");

    Model ourModel("obj/objects/terrain05.obj", false, NULL, 0.0, btVector3(0,-8,0),btVector3(4,4,4));

    Model ourModel2("obj/objects/plannets/smoothmoon.obj", true, new btSphereShape(btScalar(1.)), 1.0 , btVector3(0,50,0),btVector3(5,5,5));

    //Model ourModel3("obj/objects/building-fixed.obj", false, NULL, 0.0 , btVector3(12,12,-20),btVector3(1,1,1));
    Model ourModel3("obj/objects/Tree02-v3-normals.obj", false, NULL, 0.0 , btVector3(12,12,-20),btVector3(1,1,1));

    Model ourModel4("obj/objects/Tree03.obj", false, new btSphereShape(btScalar(1.)), 0.0 , btVector3(10,10,10), btVector3(1,1,1));

    Model::InitializeModelPhysicsWorld();

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

    unsigned int amount = 1000;
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed
    float radius = 15.0;
    float offsetx = 35.5f;
    float offsety = 0.5f;
    float offsetz = 35.5f;
    for(unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offsetx * 100)) / 100.0f - offsetx;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offsety * 100)) / 100.0f - offsety;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offsetz * 100)) / 100.0f - offsetz;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 0.1f, 0.0f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for(unsigned int i = 0; i < ourModel4.meshes.size(); i++)
    {
        unsigned int VAO = ourModel4.meshes[i].VAO;
        glBindVertexArray(VAO);
        // vertex attributes
        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
//=========== LOOP ===========================================================

    Model *currentModel = &ourModel; //current pointed-at model
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
            }
            break;
        case LOOP_MODE_EDIT :    {
            renderLoop3D(window);
            renderLoop2D(window);

            //Bullet Simulation:
            Model::RunStepSimulation();

            //draw debug stuff from bullet
            debugDraw.SetMatrices(getViewMatrix(), getprojectionMatrix());

            if(physicsDebugEnabled){
                dynamicsWorld->debugDrawWorld();
                //debugDraw.draw();
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

        //=-----=-=-=-=-==--=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-^-^-^-^-^-^-^-

            btVector3 from(cameraPos.x,cameraPos.y,cameraPos.z);
            btVector3 to(cameraPos.x+cameraFront.x*100,cameraPos.y+cameraFront.y*100,cameraPos.z+cameraFront.z*100);

            btVector3 blue(0.1, 0.3, 0.9);

            dynamicsWorld->getDebugDrawer()->drawSphere(btVector3(0,0,0), 0.5, blue); //at origin
            btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
            closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

            dynamicsWorld->rayTest(from, to, closestResults);
            if (closestResults.hasHit() && !isMouseVisable())
            {
                currentModel = ((Model *)closestResults.m_collisionObject->getCollisionShape()->getUserPointer());
                //currentModel->tint = glm::vec3(0.2,0.2,0.2);
                currentModel->selected = true;

                if(currentModel != lastModel && lastModel != NULL){
                    //lastModel->tint = glm::vec3(0,0,0);
                    lastModel->selected = false;
                }
                lastModel = currentModel;

                btVector3 p = from.lerp(to, closestResults.m_closestHitFraction);
                dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, blue);
                dynamicsWorld->getDebugDrawer()->drawLine(p, p + closestResults.m_hitNormalWorld, blue);

                //ourModel3.setPosition(p);
            }else{
                if(currentModel != NULL){
                    //currentModel->tint = glm::vec3(0,0,0);
                    currentModel->selected = false;
                }
            }

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

            //btVector3 infront((cameraPos.x + cameraFront.x), (cameraPos.y + cameraFront.y), (cameraPos.z + cameraFront.z));
            //ourModel4.setPosition(infront);

            debugDraw.draw();

        //end physics loop=====================================================================================

        //============imgui===========

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

                    printServerList(serverList);

                }

                if (ImGui::Button("exit"))
                {
                    networkLoaded = false;
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

            ourModel.Draw(ourShader, outlineShader);
            ourModel2.Draw(ourShader, outlineShader);
/*
            outlineShader.use();
            glCullFace(GL_FRONT);
            ourModel3.Draw(outlineShader,outlineShader);
            glCullFace(GL_BACK);
            ourShader.use();
*/

            ourModel3.Draw(ourShader, outlineShader);

            ourModel4.Draw(ourShader, outlineShader);

            //render imgui (render this last so it's on top of other stuff)
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            }
            break;
        case LOOP_MODE_PLAY :    {

        }
            break;
        case LOOP_MODE_LEGACY :  {
            renderLoop3D(window);
            renderLoop2D(window);

//            btVector3 from(cameraPos.x,cameraPos.y,cameraPos.z);
//            btVector3 to(cameraPos.x+cameraFront.x*100,cameraPos.y+cameraFront.y*100,cameraPos.z+cameraFront.z*100);


            instancedShader.use();
            glm::mat4 projection = getprojectionMatrix();
            glm::mat4 view = getViewMatrix();
            instancedShader.setMat4("projection", projection);
            instancedShader.setMat4("view", view);

            for(unsigned int i = 0; i < ourModel4.meshes.size(); i++)
            {




                // bind appropriate textures
                unsigned int diffuseNr  = 1;
                unsigned int specularNr = 1;
                unsigned int normalNr   = 1;
                unsigned int heightNr   = 1;
                for(unsigned int j = 0; j < ourModel4.meshes[i].textures.size(); j++)
                {
                    glActiveTexture(GL_TEXTURE0 + j); // active proper texture unit before binding
                    // retrieve texture number (the N in diffuse_textureN)
                    string number;
                    string name = ourModel4.meshes[i].textures[j].type;
                    if(name == "texture_diffuse")
                        number = std::to_string(diffuseNr++);
                    else if(name == "texture_specular")
                        number = std::to_string(specularNr++); // transfer unsigned int to stream
                    else if(name == "texture_normal")
                        number = std::to_string(normalNr++); // transfer unsigned int to stream

                     else if(name == "texture_height")
                        number = std::to_string(heightNr++); // transfer unsigned int to stream

                    // now set the sampler to the correct texture unit
                    glUniform1i(glGetUniformLocation(instancedShader.ID, (name + number).c_str()), j);
                    // and finally bind the texture
                    glBindTexture(GL_TEXTURE_2D, ourModel4.meshes[i].textures[j].id);
                }




                glBindVertexArray(ourModel4.meshes[i].VAO);
                glDrawElementsInstanced(
                    GL_TRIANGLES, ourModel4.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount
                );
            }

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

        }
            break;
        }

        // Swap buffers (write to screen)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //cleanup
    Model::Cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    cleanup3D();
    glfwTerminate();

    return 0;
}
