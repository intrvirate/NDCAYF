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

#include "networkConfig.hpp"
#include "client.hpp"

int actualSock;
struct sockaddr_in serverAddr;
char hostname[128];
int DELAY_SECS2 = 0;
int DELAY_USECS2 = 100;

int makeSocket()
{
    struct timeval tv;
    struct sockaddr_in myaddr;

    // create udp socket
    actualSock = socket(AF_INET, SOCK_DGRAM, 0);


    // create timeout for socket
    tv.tv_sec = DELAY_SECS2;
    tv.tv_usec = DELAY_USECS2;

    if (setsockopt(actualSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("opt fail");
        return -1;
    }

    return 0;
}

unsigned long long getMilliSeconds()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return ((unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000);
}


void composeMsg(char msg[], int protocol, char extra[])
{
    unsigned long long milliSeconds = getMilliSeconds();

    sprintf(msg, "%s$%s$%d$%llu$%s", SUPERSECRETKEY_CLIENT, hostname, protocol, milliSeconds, extra);
}

int makePacket(char msg[], struct packet *out, int *id)
{
    int state = 0;
    char key[128];
    char extra[256];

    strcpy(key, strtok(msg, "$"));

    if (strcmp(key, SUPERSECRETKEY_SERVER) == 0)
    {
        strcpy(out->name, strtok(NULL, "$"));
        out->ptl = std::stoi(strtok(NULL, "$"));
        out->time = std::stoull(strtok(NULL, "$"));
        strcpy(out->extra, strtok(NULL, "$"));
        strcpy(extra, out->extra);
        *id = std::stoi(strtok(extra, "&"));
    }
    else
    {
        printf("Key check failed\n");
        state = -1;
    }
    return state;
}

int connectToServer(char ip[], int *id, struct packet *msg, struct sockaddr_in *serverAddr)
{
    gethostname(hostname, 128);
    int success = 1;
    struct sockaddr_in in_addr;
    socklen_t addrlen = sizeof(in_addr);
    char buf[BUFSIZE];
    int recvlen;

    socklen_t addrlen_in = sizeof(*serverAddr);

    memset(serverAddr, 0, sizeof(*serverAddr));
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(PORT);
    serverAddr->sin_addr.s_addr = inet_addr(ip);

    char connectMsg[BUFSIZE];

    composeMsg(connectMsg, CONNECT);

    if (makeSocket() < 0)
    {
        printf("Failed to get socket");
        success = -1;
    }


    if (sendto(actualSock, connectMsg, strlen(connectMsg), 0, (struct sockaddr *)serverAddr, addrlen_in) < 0)
    {
        printf("Failed to send\n");
        success = -1;
    }

    recvlen = recvfrom(actualSock, buf, BUFSIZE, 0, (struct sockaddr *)&in_addr, &addrlen);

    printf("Recieved: %s\n", buf);

    if (makePacket(buf, msg, id) < 0)
    {
        printf("Couldn't turn into packet\n");
        success = -1;
    }

    return success;
}


void setPositions(struct entities all[], char extra[])
{
    char *ptr;
    int curId;
    bool running = true;
    //the actuall id
    ptr = strtok(extra, "&,");

    //the first element
    ptr = strtok(NULL, "&,");
    while (ptr != NULL)
    {
        curId = std::stoi(ptr);
        printf("id %s\n", ptr);

        ptr = strtok(NULL, "&,");
        printf("x %s\n", ptr);
        all[curId].x = std::stoi(ptr);

        ptr = strtok(NULL, "&,");
        printf("y %s\n", ptr);
        all[curId].y = std::stoi(ptr);

        ptr = strtok(NULL, "&,");
        printf("z %s\n", ptr);
        all[curId].z = std::stoi(ptr);

        ptr = strtok(NULL, "&,");
    }


}

void makeMouseString(glm::vec3 pos, glm::vec3 dir)
{


}
