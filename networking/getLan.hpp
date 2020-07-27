#ifndef GETLAN_H
#define GETLAN_H

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

void getInterfaces(struct ifa interfaces[], int *numFaces);
void broadcastAllInterfaces(int sock, struct ifa interfaces[], int elements, char name[]);
void getResponses(int sock, struct server servers[]);
void getAllServers(struct server servers[]);
int makeBroadcastSocket();

#endif
