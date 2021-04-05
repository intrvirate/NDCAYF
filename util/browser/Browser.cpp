#include "util/browser/Browser.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <bits/stdc++.h>

Browser::Browser(): Browser("./") { }

Browser::Browser(std::string start_dir): Browser(start_dir, false) { }

Browser::Browser(std::string start_dir, bool can_cancel) {
    setPath(start_dir);
    setSelection(start_dir);
    setCanCancel(can_cancel);

    _has_selected = false;
    _cached = false;

    // Default input box to nothing
    _input[0] = '\0';

    // IMGUI initialization
    _title = "File Browser";
    _window_flags = 0;
    _window_flags |= ImGuiWindowFlags_NoScrollbar;

    _file_window_flags = 0;
}

bool Browser::hasSelected() const {
    return _has_selected;
}

void Browser::setCanCancel(bool can_cancel) {
    _can_cancel = can_cancel;
}

bool Browser::canCancel() const {
    return _can_cancel;
}

bool Browser::hasCanceled() const {
    return _has_canceled;
}

void Browser::draw() {

    if (!_cached)
    {
        getFiles();
    }

    ImGui::Begin(_title.c_str(), NULL, _window_flags);

    // Display current path
    ImGui::Text(getPath().c_str());

    // File List section
    ImGui::BeginChild(getPath().c_str(),
            ImVec2(ImGui::GetWindowContentRegionWidth() , 150), false,
            _file_window_flags);

    int selected = -1;
    uint i = 0;
    if (getPath() == "./")
    {
        i = 1;
    }
    // Iterate through the files in this directory and display them
    for ( ; i < _files.size(); i++)
    {
        std::string file = _files[i];

        if (ImGui::Selectable(file.c_str(), i == selected))
        {
            std::cerr << "Reloading files to display\n" << std::endl;

            _cached = false;
            selected = i;
            // If the selected file is a directory, change directories
            if (file.back() == delimiter) {

                // If selected file is '..', go up a dir by removing the last
                // path item
                if (file == "../") {

                    size_t last = getPath().substr(0, getPath().size()
                        - 1).find_last_of("/");
                    setPath(getPath().substr(0,1 + last));

                }
                // else add the selection to the path
                else if (file != "./" && file != "../") {

                    setPath(getPath() + file.c_str());
                }

                setSelection("");
            }
            else {

                //pathToFile = path + file;
                setSelection(file.c_str());
            }

            // Copy the current selection path to the input box
            strcpy(_input, (getPath() + getSelection()).c_str());

            std::cerr << getPath() << getSelection() << std::endl;
        }
    }

    // File window
    ImGui::EndChild();

    // Text input box for filepath
    ImGui::InputText("", _input, IM_ARRAYSIZE(_input));

    ImGui::SameLine();

    if (ImGui::Button("Select")) {
        _has_selected = true;
    }

    if (canCancel()) {

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            _has_canceled = true;
        }
    }

    ImGui::End();
}

std::string Browser::getSelectionPath() {
    return getPath() + getSelection();
}

std::string Browser::getSelection() {
    return _current_selection;
}

void Browser::setSelection(std::string path) {
    _current_selection = path;
}

std::string Browser::getPath() {
    return _current_path;
}

void Browser::setPath(std::string path) {
    _current_path = path;
}

int Browser::getFiles() {
    struct dirent *entry;
    DIR *currentDir = opendir(getPath().c_str());

    _files.clear();


    if (currentDir == NULL)
    {
        printf("Browser::getFiles(): Can't open current dir");
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
