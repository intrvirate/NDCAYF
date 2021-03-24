#ifndef FILEUP_H
#define FILEUP_H
#include <iostream>

#include "TCP.hpp"

class FileUp: public TCP {
    public:
        FileUp(char* ip, string dir, string fileName, int type);
        void run();
    private:
        void makeHeader(string file);

        ifstream _theFile;
        struct aboutFile _fileInfo;
        string _path;


};

#endif
