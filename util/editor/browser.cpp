
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <bits/stdc++.h>

#include <iostream>
#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"

using namespace std;
vector<string> files;
string path = "./";
string pathToFile;
char delimiter = '/';
bool hasSaved = false;
bool cached = false;
string savePath = "";

int getFiles(string path)
{
    files.clear();
    struct dirent *entry;

    DIR *currentDir = opendir(path.c_str());

    if (currentDir == NULL)
    {
        printf("ERROR: Can't open current dir");
    } else
    {
        while ((entry = readdir(currentDir)) != NULL)
        {
            string file;
            struct stat stats;
            stat(entry->d_name, &stats);
            file += entry->d_name;
            unsigned char isDir = 0x4;
            if (entry->d_type == isDir)
            {
                file += "/";
            }

            files.push_back(file);
            sort(files.begin(),files.end());
        }
        closedir(currentDir);
    }

    return 0;
}

int drawBrowser(bool saving, string matches)
{
    string title;
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    // windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    if (saving)
    {
        title = "Save as:";
    } else
    {
        title = "Open:";
    }

    if (!cached)
    {
        getFiles(path);
    }

    ImVec2 windowSize;
    windowSize = ImVec2(ImGui::GetFontSize() * 20.0f, 90);
    ImGui::Begin(title.c_str(), NULL, windowFlags);

    ImGuiWindowFlags fileWindowFlags = 0;

    string pathToDisplay = "Path: " + path;
    ImGui::Text(pathToDisplay.c_str());
    ImGui::BeginChild(path.c_str(), ImVec2(ImGui::GetWindowContentRegionWidth()
        * 0.9f, 150), false, fileWindowFlags);
    int selected = -1;
    uint i = 0;
    if (path == "./")
    {
        i = 1;
    }
    for ( ; i < files.size(); i++)
    {
        string file = files[i];

        if (ImGui::Selectable(file.c_str(), i == selected))
        {
            printf("Reloading files to display\n");
            cached = false;
            selected = i;
            if (file.back() == (char)delimiter)
            {
                if (file == "../")
                {
                    size_t last = path.substr(0,path.size()
                        - 1).find_last_of("/");
                    path = path.substr(0,1 + last);
                } else if (file != "./" && file != "../")
                {
                    path += file.c_str();
                }
            } else
            {
                pathToFile = path + file;
            }
        }
    }
    ImGui::EndChild();

    string pathToFileToDisplay = "File: " + pathToFile;
    ImGui::Text(pathToFileToDisplay.c_str());

    if (saving)
    {
        ImGui::SameLine();
        static bool save = false;
        if (ImGui::Button("Save"))
        {
            printf("Got to save\n");
            hasSaved = true;
            save = true;
        }
        if (save)
        {
            savePath = pathToFile;
            printf("set savePath: %s\n", savePath.c_str());
            save = false;
        }
    }

    ImGui::End();

    return 0;
}
