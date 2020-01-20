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
#include "util/loadMenu.hpp"

#include "util/loadShaders.hpp"
#include "util/handleinput.hpp"
#include "util/otherhandlers.hpp"
#include "util/vertexGrid.hpp"

#include "util/render/render3D.hpp"

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

    //=========== SHADERS ========================================================

    // Load Shaders
    GLuint vertexShaderID = LoadVertexShader("gamedata/shaders/vertexshader.glsl");
    GLuint fragmentShaderID = LoadFragmentShader("gamedata/shaders/fragmentshader.glsl");

    GLuint shaderProgramID = LinkShaders(vertexShaderID, fragmentShaderID);
    glUseProgram(shaderProgramID);

    int incolor = glGetUniformLocation(shaderProgramID, "inColor");
    //=========== BUFFERS ========================================================

    GLuint VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
//=========== CAMERA =========================================================
    //world up vector
    const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.7f, 0.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

    glm::mat4 view;
    //lookAt(camera position, camera target, world space up vector)
    //
    view = glm::lookAt(cameraPos, cameraTarget, up);

//=========== 3D =============================================================

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    //calculated in camera section
    //glm::mat4 view = glm::mat4(1.0f);
    //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)(1024 / 768), 0.1f, 100.0f); //update screen size input


    GLuint model_location = glGetUniformLocation(shaderProgramID, "model");
    GLuint view_location = glGetUniformLocation(shaderProgramID, "view");
    GLuint projection_location = glGetUniformLocation(shaderProgramID, "projection");

    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

//=========== MAP GENERATION =================================================

    glm::vec2 rendered_center;
    GLuint rendered_center_location = glGetUniformLocation(shaderProgramID, "rendered_center");
    glUniform2f(rendered_center_location, rendered_center.x, rendered_center.y );


//=========== LOOP ===========================================================

    while( glfwWindowShouldClose(window) == 0){

        //setup
        calculateFrameTime();
        //glClearColor(0.0f, 0.0f, 0.4f, 0.0f); //default background color
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update vectors
        cameraDirection = getCameraDirection();

        glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
        glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

        cameraPos = calcCameraMovement(window, cameraPos,  cameraDirection, cameraUp);

        //convert from screen cordinates to map cordinates
        // display x => map x, display z = map y;
        rendered_center.x = (int)cameraPos.x;
        rendered_center.y = (int)cameraPos.z;

        //update matrices
        view = glm::lookAt(cameraPos, cameraPos + cameraDirection, up);
        projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 100.0f);
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.5f, 1.0f, 0.0f));

        // send matrices to shader
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform2f(rendered_center_location, rendered_center.x, rendered_center.y );

        //render
        glUseProgram(shaderProgramID);

        glUniform4f(incolor, 1.0f, 0.0f, 0.0f, 1.0f); //red
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, sizeof(indices) * 3, GL_UNSIGNED_INT, 0);

        glUniform4f(incolor, 0.0f, 1.0f, 0.0f, 1.0f); //green
        glPolygonMode(GL_FRONT_AND_BACK, returnKeysetRenderMode());
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, sizeof(indices) * 3, GL_UNSIGNED_INT, 0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();

    return 0;
}
