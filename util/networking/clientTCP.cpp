#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string>
#include <sys/time.h>
#include <glm/glm.hpp>
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

    struct pollfd pfd;
    pfd.fd = sockTCP;
    pfd.events = POLLIN | POLLHUP;
    pfd.revents = 0;

    int out;
    bool done = false;
    char buffer[2048];
    bool gotKey = false;
    while (!done)
    {//(pfd.revents == 0)
        // call poll with a timeout of 100 ms
        if (poll(&pfd, 1, 1000) > 0)
        {

            peek = recv(sockTCP, &bufT, bufTSize, MSG_PEEK | MSG_DONTWAIT);
            printf("%d peek\n", peek);

            // they broke the connection
            if (peek == 0)
            {
                printf("they hung up\n");
                return false;
            }

            // error
            if (peek < 0)
            {
                perror("msg error\n");
                return false;
            }

            // get the key
            if (!gotKey)
            {
                len = recv(readSock, buff, 2048, 0);

            }
            else
            {

            }
            done = true;
            int len;

           // char sendline[2048];
            //fgets(sendline, 2048, stdin);
            //terrain04.obj
            struct generalTCP toSend = makeBasicTCPPack(SENDINGFILE);
            struct aboutFile stuff;
            char fileName[] = "terrain04.obj";
            strcpy(stuff.name, fileName);
            stuff.type = MAP;

            printf("name %s\n", stuff.name);
            memcpy(&toSend.data, &stuff, sizeof(aboutFile));

            send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);
            printf("sended the packet\n");

            ifstream myfile;
            myfile.open("obj/objects/terrain04.obj", std::ios::out);
            string line;
            char out[5000];
            int count = 0;
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
        }
    }
    printf("do other stuff\n");
}
