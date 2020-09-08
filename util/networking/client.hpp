#ifndef CLIENT_H
#define CLIENT_H

#include <glm/glm.hpp>
//#include <libioP.h>
#include <stdarg.h>
#include <stdio.h>

// stores the key press and the direciton before key press
// and the id
struct datapoint
{
    char move[10];
    char direction[30];
    int id;
};



struct generalPack
{
    char key[10];
    char name[10];
    unsigned short int protocol;
    unsigned short int numObjects;
    struct timeval time;
    char data[1000];
};


int makeSocket();
void netLog(glm::vec3 pos, glm::vec3 front, char key[]);

int getKeyID();
int getID();
void setConnection(bool value);
bool getConnection();
struct sockaddr_in getServerAddr();
void setTestNw(bool value);
int debugPrint(const char *format, ...);


void setPositions(struct entities all[], char extra[]);
void applyDumpData(struct entities *them, char data[], int *count);
void reconcileClient(struct entities *me);

void applyKeys(char keys[], glm::vec3 dir, glm::vec3 *pos);


struct generalPack makeBasicPack(int ptl);
int send(struct generalPack toSend);
int checkServer(struct generalPack *msg);
void setServerAddr(struct sockaddr_in newServerAddr);
struct generalPack makeBasicPack(int ptl);
bool connectTo(char ip[]);

#endif
