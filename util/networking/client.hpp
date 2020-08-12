#ifndef CLIENT_H
#define CLIENT_H

#include <glm/glm.hpp>
//#include <libioP.h>
#include <stdarg.h>
#include <stdio.h>

struct MsgPacket
{
    char name[128];
    int ptl;
    unsigned long long time;
    char data[BUFSIZE];
};

// stores the key press and the direciton before key press
// and the id
struct datapoint
{
    char move[10];
    char direction[30];
    int id;
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
int connectToServer(char ip[], struct MsgPacket *msg);
int makePacket(char msg[], struct MsgPacket *out);
int makeSocket();
unsigned long long getMilliSeconds();
void setPositions(struct entities all[], char extra[]);
void netLog(std::string key, glm::vec3 front);
void setConnection(bool value);
bool getConnection();
int sendMoveData(struct datapoint point);
struct sockaddr_in getServerAddr();
int getID();
int checkServer(char buf[]);
int processMsg(char msg[], struct MsgPacket *packet);
void applyDumpData(struct entities *them, char data[], int *count);
int debugPrint(const char *format, ...);
void setTestNw(bool value);

#endif
