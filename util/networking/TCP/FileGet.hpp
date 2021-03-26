#ifndef FILEDN_H
#define FILEDN_H
#include <iostream>

#include "TCP.hpp"

class FileGet: public TCP {
    public:
        FileGet(char* ip);
        void run() override;
    private:
        bool getHeader();
        string getDir(int type, char* fileName);

        struct aboutFile _fileInfo;


};

#endif
