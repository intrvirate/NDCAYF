#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <iostream>

using namespace std;

#include "networkConfig.hpp"
#include "stream.hpp"
#include "MusicStreamer.hpp"
#include "TCP.hpp"
#include "FileGet.hpp"

FileGet::FileGet(char* ip) : TCP(ip, PORTTCP_DOWNLOAD)
{
}

string FileGet::getDir(int type, char* fileName)
{
    string dir;
    switch (type)
    {
        case MAP:
            dir = "gamedata/maps/";
            break;
        case GAMEMODE:
            dir = "gamedata/gameModes/";
            break;
    }

    dir += fileName;
    return dir;
}

bool FileGet::getHeader()
{
    bool gotIt = false;
    struct generalTCP& bufIn = getInBuf();

    while (!gotIt)
    {
        if (getFromPoll(true) == 0)
        {
            if (bufIn.protocol == SENDINGFILEHEADER)
            {
                memcpy(&_fileInfo, &bufIn.data, sizeof(struct aboutFile));

                printf("Name %s\n", _fileInfo.name);
                printf("Type %d\n", _fileInfo.type);
                printf("Size %ld\n", _fileInfo.size);

                printf("%s\n", getDir(_fileInfo.type, _fileInfo.name).c_str());

                printf("Ready to recieve file\n");
                gotIt = true;
                sendPTL(NEXTLINE);
            }
        }
    }

    return gotIt;
}

void FileGet::run()
{
    if (!validate())
        printf("oh no!\n");

    if (!getHeader())
        printf("oh no header!\n");

    string filepath = getDir(_fileInfo.type, _fileInfo.name);
    cout << "Saving to: " << filepath.c_str() << endl;
    ofstream theFile(filepath, ios::binary);

    struct generalTCP& bufIn = getInBuf();
    int count = 0;
    bool downloading = true;
    while (downloading)
    {
        if (getFromPoll(true) == 0)
        {
            if (bufIn.protocol == SENDINGFILE)
            {
                count += bufIn.numObjects;
                //drawProgress((float)count / (float)information.lines, barWidth);

                theFile << bufIn.data;
                sendPTL(NEXTLINE);
            }
            if (bufIn.protocol == ENDDOWNLOAD)
            {
                count += bufIn.numObjects;
                //drawProgress(1.0f, barWidth);

                theFile << bufIn.data;

                // confirm exit
                printf("\nWe have finished added %d bytes\n", count);
                theFile.close();
                sendPTL(ENDDOWNLOAD);
                downloading = false;
            }

        }
    }

    printf("done\n");
}
