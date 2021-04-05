#include "util/browser/Browser.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <bits/stdc++.h>

Browser::Browser(): Browser("./") { }

Browser::Browser(std::string start_dir) {
    _current_path = start_dir;
    _has_selected = false;
    _cached = false;

    // IMGUI initialization
    _title = "File Browser";
    _window_flags = 0;
    _window_flags |= ImGuiWindowFlags_NoScrollbar;

    _file_window_flags = 0;
}

bool Browser::hasSelected() const {
    return _has_selected;
}

void Browser::draw() {

    if (!_cached)
    {
        getFiles();
    }

    ImGui::Begin(_title.c_str(), NULL, _window_flags);

    // Display current path
    ImGui::Text(getSelection().c_str());

    // File List section
    ImGui::BeginChild(getSelection().c_str(), ImVec2(ImGui::GetWindowContentRegionWidth()
        * 0.9f, 150), false, _file_window_flags);

    int selected = -1;
    uint i = 0;
    if (getSelection() == "./")
    {
        i = 1;
    }
    for ( ; i < _files.size(); i++)
    {
        std::string file = _files[i];

        if (ImGui::Selectable(file.c_str(), i == selected))
        {
            printf("Reloading files to display\n");


            _cached = false;
            selected = i;
            if (file.back() == delimiter) {

                if (file == "../") {

                    size_t last = getSelection().substr(0, getSelection().size()
                        - 1).find_last_of("/");
                    getSelection() = getSelection().substr(0,1 + last);

                } else if (file != "./" && file != "../") {

                    getSelection() += file.c_str();
                }
            }
            else {

                //pathToFile = path + file;
            }
            // Set the input box to the new filename
            //string filename = pathToFile.substr(pathToFile.find_last_of("/") + 1,
                //pathToFile.size());
            //strcpy(input, filename.c_str());
        }
    }

    ImGui::EndChild();

    ImGui::End();
}

std::string Browser::getSelection() {
    return _current_path;
}

int Browser::getFiles() {
    struct dirent *entry;
    DIR *currentDir = opendir(getSelection().c_str());

    _files.clear();


    if (currentDir == NULL)
    {
        printf("ERROR: Can't open current dir");
    } else
    {
        while ((entry = readdir(currentDir)) != NULL)
        {
            std::string file;
            struct stat stats;
            stat(entry->d_name, &stats);
            file += entry->d_name;
            unsigned char isDir = 0x4;
            if (entry->d_type == isDir)
            {
                file += "/";
            }

            _files.push_back(file);
            sort(_files.begin(), _files.end());
        }
        closedir(currentDir);
    }
    _cached = true;

    return 0;
}
