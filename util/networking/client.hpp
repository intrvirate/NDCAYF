#ifndef CLIENT_H
#define CLIENT_H

#include <glm/glm.hpp>
#include <stdarg.h>
#include <stdio.h>

struct generalPack
{
    char key[10];
    char name[10];
    unsigned short protocol;
    unsigned short numObjects;
    struct timeval time;
    char data[1000];
};


// udp
int makeSocket();

// getters and setters
bool getConnection();
void setConnection(bool newConnection);
int getID();
struct sockaddr_in getServerAddr();
void resetMoves();
void setMove(unsigned int id);
void setServerAddr(struct sockaddr_in newServerAddr);
struct generalPack makeBasicPack(int ptl);

// special
void reconcileClient(struct entities *me, struct move *server, glm::vec3 *cPos);

// basic upd sock stuff
int send(struct generalPack *toSend);
int sendTo(struct generalPack *toSend, struct sockaddr_in toAddr);
int getFrom(struct generalPack *msg, struct sockaddr_in fromAddr);
int checkServer(struct generalPack *msg);

// special udp socks
bool connectTo(char ip[]);
void netLog(glm::vec3 pos, glm::vec3 front, char key[]);

//TODO add tcp sock stuff

#endif
