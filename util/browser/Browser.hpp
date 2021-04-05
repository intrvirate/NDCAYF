#ifndef BROWSER_H
#define BROWSER_H

#include <string>
#include <vector>
#include "util/imgui/imgui.h"

class Browser {
    public:
        /**
         * Creates a new Browser window in the current working directory
         */
        Browser();

        /**
         * Creates a Browser in the given directory
         *
         * @param start_dir the directory to start in
         */
        Browser(std::string start_dir);

        /**
         * Returns true if this Browser has selected and committed to a path
         *
         * Like, has clicked "select"
         *
         * @return selection status
         */
        bool hasSelected() const;

        /**
         * Draws this Browser on the screen
         */
        void draw();

        /**
         * Returns a copy of the path selected by this browser
         *
         * @return a string representation of the selection from this Browser
         */
        std::string getSelection();

    private:
        /**
         * Gets the files on the current directory and caches them
         *
         * @return 0 if successful
         */
        int getFiles();

        // IMGUI params
        std::string _title;
        ImGuiWindowFlags _window_flags;
        ImGuiWindowFlags _file_window_flags;

        // State vars
        std::string _current_path;
        bool _has_selected;

        // A cache of the files on this level
        std::vector<std::string> _files;
        bool _cached;

        // Magic numbers
        char delimiter = '/';

};

#endif
