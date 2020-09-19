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

#include "networkConfig.hpp"
#include "client.hpp"
#include "clientTCP.hpp"

using namespace std;

int sockTCP;
struct sockaddr_in tcpServer;
socklen_t addrlen = sizeof(tcpServer);

struct generalTCP bufT;
size_t bufTSize = sizeof(struct generalTCP);

int makeTCP()
{
    int success = 0;
    if ((sockTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failed to make\n");
        success = -1;
    }

    return success;
}


// makes a packet and fills in most of the data
struct generalTCP makeBasicTCPPack(int ptl)
{
    struct generalTCP pack;
    strcpy(pack.name, hostnameGet());
    pack.protocol = ptl;
    pack.numObjects = 1;

    return pack;
}


int getLines(string file)
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

void drawProgress(double percent, int width)
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


bool tcpConnect(char ip[])
{
    hostnameSet();
    bool success = true;
    memset(&tcpServer, 0, sizeof(tcpServer));
    tcpServer.sin_family = AF_INET;
    tcpServer.sin_addr.s_addr= inet_addr(ip);
    tcpServer.sin_port =  htons(PORTTCP);

    if (connect(sockTCP, (struct sockaddr*)&tcpServer, addrlen) < 0)
    {
        perror("Connect problems\n");
        success = false;
    }

    send(sockTCP, SUPERSECRETKEY_CLIENT, sizeof(SUPERSECRETKEY_CLIENT), 0);
    struct generalTCP toSend = makeBasicTCPPack(SENDINGFILE);


    // info about the file
    struct lines bunch;
    struct aboutFile stuff;
    string fileName = "terrain00.obj";
    string dir = "obj/objects/";
    dir += fileName;
    long totalLine = getLines(dir);
    stuff.lines = totalLine;
    strcpy(stuff.name, fileName.c_str());
    stuff.type = MAP;

    printf("name %s lines %d\n", stuff.name, totalLine);

    struct timeval before;
    struct timeval after;
    struct timeval diff;
    gettimeofday(&before, NULL);

    struct pollfd pfd;
    pfd.fd = sockTCP;
    pfd.events = POLLIN | POLLHUP;
    pfd.revents = 0;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int barWidth = w.ws_col - 7;

    long count = 0;
    int peek;
    bool done = false;
    bool gotKey = false;
    bool  prepFile = false;
    bool sendingFile = false;
    char buff[2048];
    char out[200];
    int len;
    string line;
    ifstream myfile;
    while (!done)
    {//(pfd.revents == 0)
        // call poll with a timeout of 100 ms
        if (poll(&pfd, 1, 1000) > 0)
        {
            peek = recv(sockTCP, &bufT, bufTSize, MSG_PEEK | MSG_DONTWAIT);
            //printf("%d peek\n", peek);

            // they broke the connection
            if (peek == 0)
            {
                printf("they hung up\n");
                return false;
            }

            // error
            if (peek < 0)
            {
                perror("msg error");
                return false;
            }

            // get the key
            if (!gotKey)
            {
                len = recv(sockTCP, buff, 2048, 0);
                if (strcmp(buff, SUPERSECRETKEY_SERVER) == 0)
                {
                    gotKey = true;
                    printf("Got the key\n");

                    // go the file prep part
                    prepFile = true;
                }

            }

            if (gotKey)
            {
                // we won't receive anything till we send the line, so don't wait
                if (prepFile)
                {
                    // send the info, open the file
                    sendingFile = true;
                    prepFile = false;

                    memcpy(&toSend.data, &stuff, sizeof(aboutFile));
                    send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);
                    printf("Sent the info about the map\n");
                    myfile.open(dir, std::ios::out);

                    // force it to send the next line
                    bufT.protocol = NEXTLINE;

                    if (!myfile.is_open())
                    {
                        perror("Error opening the file:");
                    }
                }
                else
                {
                    len = recv(sockTCP, &bufT, bufTSize, 0);
                }


                // check that we are sending a file and that they want the next line
                if (sendingFile && (bufT.protocol == NEXTLINE))
                {
                    if (getline(myfile, line))
                    {

                        drawProgress((double)count / (double)totalLine, barWidth);
                        count++;
                        // add the \n and copy to the struct then to the pack struct
                        //sprintf(out, "%s\n", line.c_str());
                        strcpy(bunch.aLine, line.c_str());
                        memcpy(&toSend.data, &bunch, sizeof(struct lines));

                        // send
                        send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);
                        //printf("%d:::::%s", count++, line.c_str());
                    }
                    else
                    {
                        // probs done with the file
                        gettimeofday(&after, NULL);
                        timersub(&after, &before, &diff);

                        printf("\nSent all %d lines in", count);
                        //printf(" %lu%03lu.%03lu milliseconds \n", diff.tv_sec, diff.tv_usec / 1000 , diff.tv_usec % 1000);
                        printf(" %lu.%06lu seconds \n", diff.tv_sec, diff.tv_usec);

                        toSend.protocol = ENDDOWNLOAD;
                        send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);

                        myfile.close();
                        close(sockTCP);
                        printf("exit\n");
                        done = true;
                    }
                }


                /*
               // char sendline[2048];
                //fgets(sendline, 2048, stdin);
                //terrain04.obj
                char next[10];
                if (myfile.is_open())
                {
                    while (getline(myfile, line))
                    {
                            struct lines bunch;
                            sprintf(out, "%s\n", line.c_str());
                            strcpy(bunch.aLine, out);
                            memcpy(&toSend.data, &bunch, sizeof(struct lines));

                            send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);
                            printf("%d:::::%s", count++, out);

                            bool wait = true;
                            printf("waiting\n");
                            while (wait)
                            {
                                printf("getting\n");
                                len = recv(sockTCP, &bufT, bufTSize, 0);
                                printf("size %d ptl %d\n", len);
                                if (bufT.protocol == NEXTLINE)
                                {
                                    wait = false;
                                    printf("continuing\n");
                                }
                                else if (len > 0)
                                {
                                    printf("%s\n", next);
                                }
                            }
                    }
                    printf("Sent all lines\n");
                    toSend.protocol = ENDDOWNLOAD;
                    send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);
                }
                else
                {
                    printf("asdfsafdasdfasdf\n");
                }

                myfile.close();



                close(sockTCP);
                printf("exit\n");
                */
            }
        }
    }
    printf("do other stuff\n");
}
