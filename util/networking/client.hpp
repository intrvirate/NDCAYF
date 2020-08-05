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
int connectToServer(char ip[], struct packet *msg);
int makePacket(char msg[], struct packet *out);
int makeSocket();
unsigned long long getMilliSeconds();
void setPositions(struct entities all[], char extra[]);
void netLog(std::string key, glm::vec3 front);
void setConnection(bool value);
bool getConnection();
int sendMoveData(struct datapoint point);
struct sockaddr_in getServerAddr();
int getID();

#endif
