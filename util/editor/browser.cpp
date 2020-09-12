
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"

using namespace std;

int drawBrowser(bool saving)
{
    string title;
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    if (saving)
    {
        title = "Save as:";
    } else
    {
        title = "Open:";
    }

    ImVec2 windowSize;
    ImGui::Begin(title.c_str(), NULL, windowFlags);

    ImGui::End();
}
