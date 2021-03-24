#include <cstring>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <iostream>
#include <fstream>

using namespace std;

#include "networkConfig.hpp"
#include "stream.hpp"
#include "MusicStreamer.hpp"
#include "TCP.hpp"
#include "FileUpload.hpp"

FileUp::FileUp(char* ip, string dir, string fileName, int type) : TCP(ip, PORTTCP_UPLOAD)
{
    _path = dir + fileName;
    _theFile.open(_path, ios::binary);

    makeHeader(fileName);

}


void FileUp::makeHeader(string file)
{
    ifstream in_file(_path, ios::binary);

    in_file.seekg(0, ios::end);
    _fileInfo.size = in_file.tellg();;

    strcpy(_fileInfo.name, file.c_str());
    _fileInfo.type = MAP;
}


void FileUp::run()
{
    if (!validate())
        printf("oh no!\n");

    struct generalTCP& bufIn = getInBuf();
    struct generalTCP& bufOut = getOutBuf();

    printf("Sending file header.\n");
    memcpy(&bufOut.data, &_fileInfo, sizeof(struct aboutFile));
    sendPTL(SENDINGFILEHEADER);

    struct timeval before;
    struct timeval after;
    struct timeval diff;

    gettimeofday(&before, NULL);

    int charsRead = 0;
    long count = 0;

    bool sending = true;
    printf("Starting\n");
    while (sending)
    {
        if (getFromPoll(true) == 0)
        {
            if (bufIn.protocol == NEXTLINE)
            {
                _theFile.read(bufOut.data, sizeof(bufOut.data));

                charsRead = _theFile.gcount();
                count += charsRead;
                bufOut.numObjects = charsRead;

                if (charsRead == sizeof(bufOut.data))
                {
                    sendPTL(SENDINGFILE);
                }
                else
                {
                    bufOut.data[charsRead] = '\0';

                    sendPTL(ENDDOWNLOAD);

                    gettimeofday(&after, NULL);
                    timersub(&after, &before, &diff);

                    printf("\nSent all %ld byes in", count);
                    printf(" %lu.%06lu seconds \n", diff.tv_sec, diff.tv_usec);

                    _theFile.close();
                }
            }
            else if (bufIn.protocol == ENDDOWNLOAD)
            {
                timersub(&bufIn.time, &before, &diff);
                printf("Server took %lu.%06lu seconds to finish\n", diff.tv_sec, diff.tv_usec);

                sending = false;
            }
        }
    }

    printf("Exit\n");
}

