
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

#include <iostream>
#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"

using namespace std;
vector<string> files;
string path = "./";
string pathToFile;
char delimiter = '/';

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
        }
        closedir(currentDir);
    }

    return 0;
}

int drawBrowser(bool saving)
{
    string title;
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    if (saving)
    {
        title = "Save as:";
    } else
    {
        title = "Open:";
    }

    getFiles(path);

    ImVec2 windowSize;
    ImGui::Begin(title.c_str(), NULL, windowFlags);
    int selected = -1;
    for ( uint i=0; i < files.size(); i++)
    {
        string file = files[i];

        if (ImGui::Selectable(file.c_str(), i == selected))
        {
            printf("%d\n",selected);
            selected = i;
            if (file.back() == (char)delimiter)
            {
                path += file.c_str();
            } else
            {
                pathToFile = path + file;
            }
            printf("path: %s\n",path.c_str());
            printf("pathToFile: %s\n",pathToFile.c_str());
        }
    }

    ImGui::End();

    return 0;
}
