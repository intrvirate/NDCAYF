#ifndef BROWSER_H
#define BROWSER_H

#define INPUT_SIZE 256

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
         * Creates a Browser in the given directory
         *
         * @param start_dir     the directory to start in
         * @param can_cancel    whether or not this Browser can be dismissed
         */
        Browser(std::string start_dir, bool can_cancel);

        /**
         * Returns true if this Browser has selected and committed to a path
         *
         * Like, has clicked "select"
         *
         * @return selection status
         */
        bool hasSelected() const;

        /**
         * Sets the ability for the user to cancel selection
         *
         * @param can_cancel    true if can cancel
         */
        void setCanCancel(bool can_cancel);

        /**
         * Returns true if this Browser can be canceled
         *
         * @return true if this Browser can be canceled
         */
        bool canCancel() const;

        /**
         * Returns true if this Browser has been canceled
         *
         * @return true if this Browser has been canceled
         */
        bool hasCanceled() const;

        /**
         * Draws this Browser on the screen
         */
        void draw();

        /**
         * Returns a copy of the path to the filename selected by this browser
         *
         * @return a string representation of the selection from this Browser
         */
        std::string getSelectionPath();

        /**
         * Returns a copy of the filename selected by this browser
         *
         * @return a string representation of the selection from this Browser
         */
        std::string getSelection();

        /**
         * Sets the path of this Browser to a new path
         *
         * @param path  the path, '/' delimited.
         */
        void setSelection(std::string path);

        /**
         * Returns the path that this browser is on
         *
         * @return a string representation of the working dir of this browser
         */
        std::string getPath();

        /**
         * Sets the current path of this Browser
         *
         * @param path  the path, '/' delimited.
         */
        void setPath(std::string path);

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
        std::string _current_selection;
        char _input[INPUT_SIZE];
        bool _has_selected;
        bool _can_cancel;
        bool _has_canceled;

        // A cache of the files on this level
        std::vector<std::string> _files;
        bool _cached;

        // Magic numbers
        char delimiter = '/';
};

#endif
