#include "startupConsole.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <GLFW/glfw3.h>
#include "main.hpp"
#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"
#include <unistd.h>

using namespace std;

//global message array
#define MESSAGE_LENGTH 30
message messageList[MESSAGE_LENGTH];
uint messageIndex = 0; //points to current top message;
uint numMesages = 0;   //number of messages so far printed, maxes out at MESSAGE_LENGTH
bool fatalError = false;

//renders 1 opengl frame when called
void updateStartupConsole(){
    if(glfwWindowShouldClose(window)){
        exit(0); //todo: cleanup
    }
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui::SetNextWindowSize(ImVec2(display_w, display_h));
    ImGui::SetNextWindowPos(ImVec2(0.0f,0.0f));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoBackground;
    window_flags |= ImGuiWindowFlags_NoResize;
    ImGui::Begin("console", NULL, window_flags);

    if(numMesages == MESSAGE_LENGTH){
        for(uint i = messageIndex; i < MESSAGE_LENGTH; i++){
            printFormatedString(messageList[i]);
        }
    }
    for(uint i = 0; i < messageIndex; i++){
        printFormatedString(messageList[i]);
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();

    if(fatalError)
        waitToCloseError();
}

void printFormatedString(message message){
    string printstr;
    switch (message.type){
    case ERROR:
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ERROR: "); ImGui::SameLine();
        break;
    case WARNING:
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "WARNING: ");ImGui::SameLine();
        break;
    case SUCCESS:
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SUCCESS: ");ImGui::SameLine();
        break;
    case STATUS:
        ImGui::Text("> ");ImGui::SameLine();
        break;
    case FATALERROR:
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "FATAL ERROR: ");ImGui::SameLine();
        fatalError = true;
        break;
    case NONE:
        break;
    }
    ImGui::Text("%s", message.message.c_str());
}

void printMessage(string messageString, const char* str,  messageType type){
    messageString += str;
    printMessage(messageString, type);
}

void printMessage(string messageString, const char* str, const char* str2, messageType type){
    messageString += str;
    messageString += " ";
    messageString += str2;
    printMessage(messageString, type);
}


void printMessage(string messageString, messageType type){
    messageList[messageIndex].type = type;
    messageList[messageIndex].message = messageString;

    messageIndex++;
    if(messageIndex == MESSAGE_LENGTH )
        messageIndex = 0;

    numMesages < MESSAGE_LENGTH ? numMesages++ : 0;

    updateStartupConsole();
}

void updateMessage(string messageString){
    if(messageIndex != 0 && messageIndex < MESSAGE_LENGTH)
        messageList[messageIndex - 1].message += messageString;
    if(messageIndex == 0)
        messageList[MESSAGE_LENGTH - 1].message += messageString;

    updateStartupConsole();
}

//only allows changing a status type to a more specific type
void updateMessageType(messageType type){
    if(messageIndex != 0 && messageIndex < MESSAGE_LENGTH)
        if(messageList[messageIndex - 1].type == STATUS)
            messageList[messageIndex - 1].type = type;
    if(messageIndex == 0)
        if(messageList[MESSAGE_LENGTH - 1].type == STATUS)
            messageList[MESSAGE_LENGTH - 1].type = type;

}

void waitToCloseError(){
    while(1){
        updateStartupConsole();
        usleep(100);
    }
}
