#ifndef BROWSER_H
#define BROWSER_H

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
        void draw() const;

        /**
         * Returns the path selected by this browser
         *
         * @return a string representation of the selection from this Browser
         */
        std::string getSelection() const;

    private:
        std::string _currentPath;
        bool _hasSelected;
};

#endif
