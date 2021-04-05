#include "util/browser/Browser.hpp"

Browser::Browser(): Browser("./") { }

Browser::Browser(std::string start_dir) {
    _current_path = start_dir;
    _has_selected = false;

    // IMGUI initialization
    _title = "File Browser";
    _window_flags = 0;
    _window_flags |= ImGuiWindowFlags_NoScrollbar;

    _window_size = ImVec2(ImGui::GetFontSize() * 20.0f, 90);
}

bool Browser::hasSelected() const {
    return _has_selected;
}

void Browser::draw() {
    // TODO
    ImGui::Begin(_title.c_str(), NULL, _window_flags);

    ImGui::End();
}

std::string Browser::getSelection() {
    return _current_path;
}
