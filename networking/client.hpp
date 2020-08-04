#ifndef CLIENT_H
#define CLIENT_H

#include <glm/glm.hpp>

struct packet
{
    char name[128];
    int ptl;
    unsigned long long time;
    char extra[BUFSIZE];
};

/*
struct server {
    struct sockaddr_in routes[5];
    int numRoutes;
    char name[128];
    bool hasLo;
    int loIndex;
};
*/

void composeMsg(char msg[], int protocol, char extra[] = "none");
int connectToServer(char ip[], int *id, struct packet *msg, struct sockaddr_in *serverAddr);
int makePacket(char msg[], struct packet *out);
int makeSocket();
unsigned long long getMilliSeconds();
void setPositions(struct entities all[], char extra[]);

#endif
