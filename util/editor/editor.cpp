#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <iostream>
#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"

#include "util/json.hpp"

#include "util/object/object.h"
#include "util/handleinput.hpp"


void drawEditor()
{
    if(showProperties)
    {

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_NoResize;

        ImGui::Begin("Properties", NULL, window_flags);
        ImGui::Text("Hello, World!");

        ImGui::End();
    }

}
