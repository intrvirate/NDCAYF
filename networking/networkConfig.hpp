#ifndef NETWORK_H
#define NETWORK_H

/*
 * store all network constants here,
 * or elsewhere once integrated into
 * the main code, ikc
 *
 */
#include <glm/glm.hpp>

#define PORT  12345
#define SUPERSECRETKEY_SERVER "ndcayfserver"
#define SUPERSECRETKEY_CLIENT "ndcayfclient"
#define PONG 1
#define PING 2
#define STATE 3
#define SPEED 4
#define CONNECT 5
#define MOVE 6
#define DUMP 7

#define MAXSERVERS 5

// not sure if this value is important enough
#define BUFSIZE 2048


struct ifa {
    char name[128];
    char ip[128];
};

struct server {
    struct sockaddr_in routes[5];
    int numRoutes;
    char name[128];
    bool hasLo;
    int loIndex;
};

struct entities
{
    int x;
    int y;
    int z;
};

#endif
