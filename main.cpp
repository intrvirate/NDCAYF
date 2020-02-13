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


using json = nlohmann::json;
using namespace std;
using namespace glm;
GLFWwindow* window;

int main()
{
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
    window = glfwCreateWindow( CurrentWindowX, CurrentWindowY, "NDCAYF", NULL, NULL); //create window
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. Double check that the GPU is 3.3 compatible\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
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


    Model ourModel("obj/objects/terrain03.obj", true, new btSphereShape(btScalar(1.)), 0.0, btVector3(-1,5,0));

    Model ourModel2("obj/objects/plannets/moon.obj", true, new btSphereShape(btScalar(1.)), 0.0 , btVector3(2,1,1));

    Model ourModel3("obj/objects/building02.obj", true, new btSphereShape(btScalar(1.)), 1 , btVector3(4,30,0));

    Model::InitializeModelPhysicsWorld();

//=========== BULLET =========================================================
//TODO: mmove this out of main, this is just for testing



    //see the helloWorld bullet example file for documentation on these functions
    //TODO: use a multithreaded initializer instead of this single threaded one


    //btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    //btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    //btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    //btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    //btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    //dynamicsWorld->setGravity(btVector3(0, -5, 0));

    //set up debugging view
    //BulletDebugDrawer_OpenGL debugDraw;
    //debugDraw.loadDebugShaders();
    //dynamicsWorld->setDebugDrawer(&debugDraw);

    //btAlignedObjectArray<btCollisionShape*> collisionShapes;

    ///create a few basic rigid bodies

    //the ground is a cube of side 100 at position y = -56.
    //the sphere will hit it at y = -6, with center at -5

    {
        //btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
        btCollisionShape* groundShape = new btSphereShape(btScalar(15.));
        collisionShapes.push_back(groundShape);

        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, 0, 0));

        btScalar mass(0.);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            groundShape->calculateLocalInertia(mass, localInertia);

        //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        //add the body to the dynamics world
        dynamicsWorld->addRigidBody(body);
    }
btRigidBody* body ;
    {
        //create a dynamic rigidbody

        //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
        btCollisionShape* colShape = new btSphereShape(btScalar(1.2));
        collisionShapes.push_back(colShape);

        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        btScalar mass(1.f);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            colShape->calculateLocalInertia(mass, localInertia);

        startTransform.setOrigin(btVector3(5, 40, 1));

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
         body = new btRigidBody(rbInfo);

        dynamicsWorld->addRigidBody(body);


    }
;
//=========== RENDER =========================================================

    load3DShaders();
    load3DBuffers();
    load3DMatrices();

    load2DShaders();
    load2DBuffers();
    loadTextDataSpacing();

    loadAutoMapGen();

//=========== LOOP ===========================================================


    while( glfwWindowShouldClose(window) == 0){

        //setup
        calculateFrameTime();
        //glClearColor(0.0f, 0.0f, 0.4f, 0.0f); //default background color
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderLoop3D(window);
        renderLoop2D(window);

        //ASSIMP render loop (not cleaned up yet)

        //Bullet Simulation:
        Model::RunStepSimulation();
        //dynamicsWorld->stepSimulation(getFrameTime(), 10);

        glm::mat4 modelPhys = glm::mat4(1.0f);

        /*
        for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
        {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
            btRigidBody* body = btRigidBody::upcast(obj);
            btTransform trans;
            if (body && body->getMotionState())
            {
                body->getMotionState()->getWorldTransform(trans);
            }
            else
            {
                trans = obj->getWorldTransform();
            }

            //obj->getWorldTransform().getOpenGLMatrix(glm::value_ptr(modelPhys));
            modelPhys = glm::translate(modelPhys, glm::vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ())));
            //modelPhys = glm::scale(modelPhys, glm::vec3(1.0f, 1.0f, 1.0f));

        }
*/

        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1]; //moon is object 1
        body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(modelPhys));  //get it's transform matrix


        //draw debug stuff from bullet
        debugDraw.SetMatrices(getViewMatrix(), getprojectionMatrix());
        dynamicsWorld->debugDrawWorld();


//end physics loop=====================================================================================

        ourShader.use();

        glm::mat4 projection = getprojectionMatrix();
        glm::mat4 view = getViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -7.0f, 0.0f));
        model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        //moonobj->getWorldTransform().getOpenGLMatrix(glm::value_ptr(modelPhys));
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(3.0f, 60.0f, 3.0f));
        //model = glm::scale(model, glm::vec3(12.9f, 12.9f, 12.9f));
        //ourShader.setMat4("model", model);
        ourShader.setMat4("model", modelPhys);
        ourModel2.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-14.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        ourShader.setMat4("model", model);
        ourModel3.Draw(ourShader);


        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    //draw model physics debug
    Model::DrawDebugModels();


    //cleanup physics code
    //TODO: clean this up
    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

    //delete collision shapes
    /*
    for (int j = 0; j < collisionShapes.size(); j++)
    {
        btCollisionShape* shape = collisionShapes[j];
        collisionShapes[j] = 0;
        delete shape;
    }
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    collisionShapes.clear();

*/
    Model::Cleanup();

    cleanup3D();
    glfwTerminate();

    return 0;
}
