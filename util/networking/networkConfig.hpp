#ifndef NETWORK_H
#define NETWORK_H
#include <glm/glm.hpp>

/*
 * store all network constants here,
 * or elsewhere once integrated into
 * the main code, ikc
 *
 */
#define PORT  12345
#define SUPERSECRETKEY_SERVER "ndcayfser"
#define SUPERSECRETKEY_CLIENT "ndcayfcli"
#define PONG 1
#define PING 2
#define STATE 3
#define SPEED 4
#define CONNECT 5
#define MOVE 6
#define DUMP 7

#define IPLEN 30

#define SERVERDUMPSPERSEC 6
#define KEYUPDATE 60

#define MAXSERVERS 5
#define MAXPLAYERS 20

// not sure if this value is important enough
#define BUFSIZE 2048

// a universal key map that all clients will understand 
#define UNI_FD "w"
#define UNI_BK "s"
#define UNI_RT "d"
#define UNI_LT "a"


struct ifa {
    char name[128];
    char ip[128];
};

struct server {
    char routes[5][100];
    int numRoutes;
    char name[128];
    bool hasLo;
    int loIndex;
};

struct move
{
    glm::vec3 pos;
    glm::vec3 dir;
    char extraActions[5];
};

struct entities
{
    struct move moves[60];
    unsigned short numMoves;
    unsigned int moveID;
};

extern bool connected;

#endif
