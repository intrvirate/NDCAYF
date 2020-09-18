#ifndef NETWORK_H
#define NETWORK_H
#include <glm/glm.hpp>

/*
 * store all network constants here,
 * or elsewhere once integrated into
 * the main code, idc
 *
 */
#define PORT  12345
#define PORTTCP  54321
#define SUPERSECRETKEY_SERVER "ndcayfser"
#define SUPERSECRETKEY_CLIENT "ndcayfcli"
#define PONG 1
#define PING 2
#define STATE 3
#define SPEED 4
#define CONNECT 5
#define MOVE 6
#define DUMP 7
#define INFO 8
#define SENDINGFILE 9
#define NEXTLINE 10
#define ENDDOWNLOAD 11

#define MAP 1
#define GAMEMODE 2
#define OBJ 3
#define TEXTURE 4

#define UDP_TV_SEC 0
#define UDP_TV_USEC 100

#define IPLEN 30

#define SERVERDUMPSPERSEC 6
#define KEYUPDATE 60

#define MAXSERVERS 5
#define MAXPLAYERS 20

struct ifa {
    char name[128];
    char ip[128];
};

struct move
{
    glm::vec3 pos;
    glm::vec3 dir;
    char extraActions[5];
};

struct Unvalid
{
    struct move theMove;
    int id;
};

struct entities
{
    struct move moves[60];
    unsigned short numMoves;
    unsigned int moveID;
};

struct infoStruct
{
    unsigned short maxPlayers;
    unsigned short curPlayers;
    char mapName[25];
    char gameType[25];
    bool isCustom;
};

struct server {
    char routes[5][100];
    int numRoutes;
    char name[128];
    bool hasLo;
    int loIndex;
    struct infoStruct about;
};

#endif
