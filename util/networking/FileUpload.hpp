#ifndef FILEUP_H
#define FILEUP_H
#include <iostream>

#include "TCP.hpp"

class Upload: public TCP {
    public:
        Upload(char* ip, string dir, string fileName, int type);
        void run();
    private:
        void makeHeader(string file);

        struct aboutFile _fileInfo;
        string _path;


};

#endif
