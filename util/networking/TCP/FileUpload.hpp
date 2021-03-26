#ifndef FILEUP_H
#define FILEUP_H
#include <iostream>

#include "TCP.hpp"

class FileUpload: public TCP {
    public:
        FileUpload(char* ip, string dir, string fileName, int type);
        void run() override;
    private:
        void makeHeader(string file);

        struct aboutFile _fileInfo;
        string _path;


};

#endif
