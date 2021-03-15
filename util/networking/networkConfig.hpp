#ifndef NETWORK_H
#define NETWORK_H
#include <glm/glm.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

/*
 * store all network constants here,
 * or elsewhere once integrated into
 * the main code, idc
 *
 */
#define PORT  12345
#define PORTTCP_UPLOAD  54321
#define PORTTCP_DOWNLOAD  54322
#define PORTTCP_MUSIC  54323
#define PORTTCP_VOICE  54324
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
#define STARTSTREAM 12
#define MORESONG 13
#define SONGHEADER 14
#define ENDSONG 15

#define UPLOADFILE 1
#define DOWNLOADFILE 2
#define STREAMMUSIC 3
#define STREAMVOICE 4

#define NUM_BUFFERS 32
#define MUSIC_BUFFERS 105
#define PACKET_DATA 6000
#define BUFFER_SIZE 6000

#define POLLOK 0
#define POLLHUNGUP 1
#define POLLBAD -1

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

struct musicHeader
{
    uint8_t channels;
    int32_t sampleRate;
    uint8_t bitsPerSample;
    ALsizei dataSize;
    ALenum format;
};
#endif
