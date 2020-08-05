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

int keyID = 0;
int clientID;
struct datapoint unvalidated[100];
int unvalidSize = 0;
bool connected = false;

void setConnection(bool value)
{
    connected = value;
}

bool getConnection()
{
    return connected;
}

struct sockaddr_in getServerAddr()
{
    return serverAddr;
}

int getID()
{
    return keyID;
}

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

    sprintf(msg, "%s$%s$%d$%llu$%d$%s", SUPERSECRETKEY_CLIENT, hostname, protocol, milliSeconds, clientID, extra);
}

int makePacket(char msg[], struct packet *out)
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
        clientID = std::stoi(strtok(extra, "&"));
    }
    else
    {
        printf("Key check failed\n");
        state = -1;
    }
    return state;
}

int connectToServer(char ip[], struct packet *msg)
{
    gethostname(hostname, 128);
    int success = 1;

    struct sockaddr_in in_addr;
    socklen_t addrlen = sizeof(in_addr);
    char buf[BUFSIZE*2];
    int recvlen = -1;

    socklen_t addrlen_in = sizeof(serverAddr);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    char connectMsg[BUFSIZE];

    composeMsg(connectMsg, CONNECT);

    if (makeSocket() < 0)
    {
        printf("Failed to get socket");
        success = -1;
    }


    if (sendto(actualSock, connectMsg, strlen(connectMsg), 0, (struct sockaddr *)&serverAddr, addrlen_in) < 0)
    {
        printf("Failed to send\n");
        success = -1;
    }

    // actually get the response
    int tries = 0;
    while (recvlen < 0)
    {
        recvlen = recvfrom(actualSock, buf, BUFSIZE, 0, (struct sockaddr *)&in_addr, &addrlen);
        tries++;
        if (tries > 1000)
        {
            success = -1;
            printf("Failed to connect server\n");
        }
    }

    if (!(success < 0))
    {
        printf("Recieved: %s\n", buf);

        if (makePacket(buf, msg) < 0)
        {
            printf("Couldn't turn into packet\n");
            success = -1;
        }
    }

    return success;
}

int sendMoveData(struct datapoint point)
{
    char data[2048];
    char msg[BUFSIZE];
    int success = 1;

    sprintf(data, "%s&%s&%d", point.direction, point.move, point.id);
    socklen_t addrlen_in = sizeof(serverAddr);


    composeMsg(msg, MOVE, data);
    printf("Sending: %s\n", msg);


    if (sendto(actualSock, msg, strlen(msg), 0, (struct sockaddr *)&serverAddr, addrlen_in) < 0)
    {
        printf("Failed to send\n");
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
        all[curId].cameraPos.x = std::stof(ptr);

        ptr = strtok(NULL, "&,");
        printf("y %s\n", ptr);
        all[curId].cameraPos.y = std::stof(ptr);

        ptr = strtok(NULL, "&,");
        printf("z %s\n", ptr);
        all[curId].cameraPos.z = std::stof(ptr);

        ptr = strtok(NULL, "&,");
    }


}


void netLog(std::string key, glm::vec3 front)
{
    // separates each key by , useless
    /*
    int len = key.length();
    for (int i = 0; i < len; i++)
    {
        key.insert(i*2 + 1, ",");
    }
    key.pop_back();

    //key.insert(0, "(");
    //key.append(")");
    */

    strcpy(unvalidated[unvalidSize].move, key.c_str());

    char dir[30];
    sprintf(dir, "%.2f,%.2f,%.2f", front.x, front.y, front.z);

    strcpy(unvalidated[unvalidSize].direction, dir);

    unvalidated[unvalidSize].id = keyID++;

    //printf("Key: [%s], dir: [%s], [%d]\n", unvalidated[unvalidSize].move, unvalidated[unvalidSize].direction, unvalidated[unvalidSize].id);

    if (sendMoveData(unvalidated[unvalidSize]) < 0)
    {
        printf("Something went wrong with key send\n");
    }
}
