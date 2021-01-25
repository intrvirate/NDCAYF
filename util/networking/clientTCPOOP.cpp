#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <string>
#include <poll.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "networkConfig.hpp"
#include "client.hpp"
#include "clientTCPOOP.hpp"

int TCP::makeTCP()
{
    int success = 0;
    if ((sockTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failed to make\n");
        success = -1;
    }

    return success;
}

TCP::TCP(char* ip, int type, string file)
{
    addrlen = sizeof(tcpServer);
    fileName = file;


    // amke the socket
    makeTCP();

    // try and connnect to the server
    if (!tcpConnect(ip))
    {
        printf("tcpError!\n");
        exit(EXIT_FAILURE);
    }


    // universal stuff
    gettimeofday(&before, NULL);

    // for the poll
    pfd.fd = sockTCP;
    pfd.events = POLLIN | POLLHUP;
    pfd.revents = 0;

    charsRead = 0;
    charsProcessed = 0;

    count = 0;
    done = false;

    bufTSize = sizeof(struct generalTCP);


    // verify they are server
    if (!TCP::waitForKey())
    {
        perror("Failed to find key!\n");
        exit(EXIT_FAILURE);
    }

    // type specific set up and run
    if (type == UPLOADFILE)
    {
        fileSendInit();
        printf("Uploading file!\n");
        fileSendMain();
    }
    else if (type == DOWNLOADFILE)
    {
        printf("Downloading file!\n");
        exit(EXIT_FAILURE);
    }
    else if (type == STREAMMUSIC)
    {
        printf("Streaming music\n");
    }
    else if (type == STREAMVOICE)
    {
        printf("Voice channel\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * inits file send specific vars and such
 * @param fileName the name of the file we are to open
 */
void TCP::fileSendInit()
{
    toSend = makeBasicTCPPack(SENDINGFILE);
    string dir = "obj/objects/";
    printf("%s..%s\n", dir.c_str(), fileName.c_str());
    fileName = dir + fileName;
    totalLine = getLines(fileName);

    ifstream in_file(fileName, ios::binary);
    in_file.seekg(0, ios::end);
    int fileSize = in_file.tellg();

    // file stuff
    fileInfo.lines = fileSize;
    strcpy(fileInfo.name, fileName.c_str());
    fileInfo.type = MAP;

    printf("name %s lines %ld\n", fileInfo.name, totalLine);


    // for the progress bar
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    barWidth = w.ws_col - 8;

    sendingFile = false;
    waitingForTime = false;
}

/**
 * waits for poll to trigger, then error checks, and sets the buf packet
 * 0 for fine, 1 for hung up and -1 for bad
 * @param waitForFill to wait for the buffer to fill up or not
 * @return int of what happened
 */
int TCP::getFromPoll(bool waitForFill)
{
    int peek;
    if (poll(&pfd, 1, 1000) > 0)
    {
        peek = recv(sockTCP, &bufT, bufTSize, MSG_PEEK | MSG_DONTWAIT);
        //printf("%d peek\n", peek);

        // they broke the connection
        if (peek == 0)
        {
            printf("they hung up\n");
            return POLLHUNGUP;
        }

        // error
        if (peek < 0)
        {
            perror("msg error");
            return POLLBAD;
        }

        if (waitForFill)
        {
            if (peek < sizeof(bufT))
            {
                //printf("waiting for it all\n");
                bufT.protocol = -1;
            }
            else
            {
                len = recv(sockTCP, &bufT, bufTSize, 0);
            }
        }
    }

    return POLLOK;
}

/**
 * waites for server to send the key
 * @return if we got one
 * TODO make this only try x amount of times before failing
 */
bool TCP::waitForKey()
{
    bool waiting = true;
    bool success = false;
    char buff[2048];
    while (waiting)
    {
        if (getFromPoll(false) == 0)
        {
            len = recv(sockTCP, buff, 2048, 0);
            if (strcmp(buff, SUPERSECRETKEY_SERVER) == 0)
            {
                success = true;
                waiting = false;
                printf("Got the key\n");
            }
        }
    }

    len = 0;
    return success;
}

/**
 *
 * @param myfile
 * @return
 */
void TCP::sendFileInfo(ifstream &myfile)
{
    memcpy(&toSend.data, &fileInfo, sizeof(aboutFile));
    send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);
    printf("Sent the info about the map\n");
    myfile.open(fileName, std::ios::out);

    if (!myfile.is_open())
    {
        perror("Error opening the file:");
    }
}

/**
 * sends the next line
 * if the buffer fills up then nothing special
 * but if not then we send with ENDDOWNLOAD ptl
 * @return
 */
bool TCP::sendNextLine(ifstream &myfile)
{
    // fillup the buffer with info from the file
    if (myfile.read(toSend.data, sizeof(toSend.data)))
    {
        // it filled up correctly
        charsRead = myfile.gcount(); // should be 1000
        count += charsRead;
        toSend.numObjects = charsRead;
        drawProgress((double)count / (double)fileInfo.lines, barWidth);


        // send
        if (send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0) < 0)
        {
            perror("send wackiness");
        }
    }
    else
    {
        // the buffer did not fully fill, at the end

        // should be <1000
        charsRead = myfile.gcount();
        count += charsRead;
        toSend.numObjects = charsRead;

        drawProgress(1.0f, barWidth);

        //printf(" %lu%03lu.%03lu milliseconds \n", diff.tv_sec, diff.tv_usec / 1000 , diff.tv_usec % 1000);

        toSend.protocol = ENDDOWNLOAD;
        send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);

        gettimeofday(&after, NULL);
        timersub(&after, &before, &diff);

        printf("\nSent all %ld byes in", count);
        printf(" %lu.%06lu seconds \n", diff.tv_sec, diff.tv_usec);

        myfile.close();

        waitingForTime = true;
        sendingFile = false;
    }
}

/**
 * main loop for sending files
 * assumes the key has been got
 * preps file and sends the first line before entering
 * @return
 */
bool TCP::fileSendMain()
{
    ifstream myfile;
    bool done = false;

    // prep file and send
    sendFileInfo(myfile);

    bool first = true;
    sendingFile = true;

    printf("starting\n");
    while (!done)
    {
        if (getFromPoll(true) == 0)
        {
            if (first)
            {
                sendNextLine(myfile);
                first = false;
            }
            // check that we are sending a file and that they want the next line
            if (sendingFile && (bufT.protocol == NEXTLINE))
            {
                sendNextLine(myfile);
            }
            else if (waitingForTime && (bufT.protocol == ENDDOWNLOAD))
            {
                timersub(&bufT.time, &before, &diff);
                printf("Server took %lu.%06lu seconds to finish\n", diff.tv_sec, diff.tv_usec);

                done = true;
                printf("exit\n");
                close(sockTCP);
            }
        }
    }

    return true;
}


// makes a packet and fills in most of the data
struct generalTCP TCP::makeBasicTCPPack(int ptl)
{
    struct generalTCP pack;
    strcpy(pack.name, hostnameGet());
    pack.protocol = ptl;
    pack.numObjects = 1;

    return pack;
}


int TCP::getLines(string file)
{
    ifstream fileToCount;
    int count = 0;
    string line;

    fileToCount.open(file);

    while (getline(fileToCount, line))
    {
        count++;
    }

    fileToCount.close();

    return count;
}

void TCP::drawProgress(double percent, int width)
{
    cout << "[";
    int pos = width * percent;
    for (int i = 0; i < width; ++i) {
        if (i < pos) cout << "=";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << int(percent * 100.0) << " %\r";
    cout.flush();
}


/**
 * connects our tcp socket to the server, sends our key
 * @param ip the server ip
 * @return if we did it
 */
bool TCP::tcpConnect(char ip[])
{
    hostnameSet();
    bool success = true;
    memset(&tcpServer, 0, sizeof(tcpServer));
    tcpServer.sin_family = AF_INET;
    tcpServer.sin_addr.s_addr= inet_addr(ip);
    tcpServer.sin_port =  htons(PORTTCP);

    // try to connect, if yes then send the key
    if (connect(sockTCP, (struct sockaddr*)&tcpServer, addrlen) < 0)
    {
        perror("Connect problems\n");
        success = false;
    }
    else
    {
        send(sockTCP, SUPERSECRETKEY_CLIENT, sizeof(SUPERSECRETKEY_CLIENT), 0);
    }

    return success;
}
