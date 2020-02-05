#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util/json.hpp"

#include "util/render/render3D.hpp"
#include "util/render/render2D.hpp"

#include "util/loadMenu.hpp"

#include "util/loadShaders.hpp"
#include "util/handleinput.hpp"
#include "util/otherhandlers.hpp"

using json = nlohmann::json;
using namespace std;
using namespace glm;
GLFWwindow* window;

int main()
{
    //=========== SETUP ==========================================================

    //do this first because settings.json will contain things like default window size
    setJsonDebugMode(true);
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
    window = glfwCreateWindow( 1024, 768, "Tutorial 02 - Red triangle", NULL, NULL); //create window
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. Double check that the GPU is 3.3 compatible\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glViewport(0, 0, 1024, 768); //should match window size
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

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    cleanup3D();
    glfwTerminate();

    return 0;
}
