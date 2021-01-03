#ifndef GETLAN_H
#define GETLAN_H

void getInterfaces(struct ifa interfaces[], int *numFaces);
void broadcastAllInterfaces(int sock, struct ifa interfaces[], int elements, char name[]);
//void getResponses(int sock, struct server servers[]);
//void getResponses(int sock, struct server servers[], struct ifa interfaces[]);
void getResponses(int sock, struct server servers[], struct ifa interfaces[], int *numFaces);
void getBroadCast(struct sockaddr_in ip, struct in_addr subnet, struct in_addr *broadcastOut);
void getAllServers(struct server servers[]);
void getAllServersa(struct server servers[]);
int makeBroadcastSocket();
void printServerList(struct server *list);
void makeServerListWindow(struct server *theList);

#endif
