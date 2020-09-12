#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include "util/imgui/imgui.h"

#include "networkConfig.hpp"
#include "getLan.hpp"
#include "client.hpp"


/*
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

*/

char lo[128];
int DELAY_SECS = 1;
int DELAY_USECS = 0;
struct generalPack infoPack = makeBasicPack(INFO);


struct pingPack
{
    char key[10];
    char name[10];
    unsigned short int protocol;
    struct timeval time;
    char data[1000];
};



void getInterfaces(struct ifa interfaces[], int *numFaces)
{
    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];

    getifaddrs(&ifaddr);

    // loops through each element in ifaddr
    while (ifaddr)
    {
        family = ifaddr->ifa_addr->sa_family;


        // checks if it is the right kind
        if (family == AF_INET) {
            s = getnameinfo(ifaddr->ifa_ifu.ifu_broadaddr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            // checks if lo
            if (strcmp(ifaddr->ifa_name, "lo") == 0)
            {
                strcpy(lo, host);
            }

            // adds to the array"before %s

            strcpy(interfaces[*numFaces].name, ifaddr->ifa_name);
            strcpy(interfaces[*numFaces].ip, host);
            (*numFaces)++;

            //printf("Interface: %-20sBroadcast: %s\n", ifaddr->ifa_name, host);

       }

        ifaddr = ifaddr->ifa_next;
    }

    // free up the space, and return the amount of elements in the array
    freeifaddrs(ifaddr);
}

void broadcastAllInterfaces(int sock, struct ifa interfaces[], int elements, char name[])
{
    struct sockaddr_in broadcast_addr;
    char msg[100];

    struct timeval tv;


    struct pingPack ping;
    gettimeofday(&ping.time, NULL);
    strcpy(ping.key, SUPERSECRETKEY_CLIENT);
    strcpy(ping.name, name);
    ping.protocol = INFO;

    socklen_t addrlen = sizeof(broadcast_addr);

    // address stuff
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);
    //broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    for (int i = 0; i < elements; i++)
    {
        broadcast_addr.sin_addr.s_addr = inet_addr(interfaces[i].ip);

        printf("Broadcast to %-15s %s\n", interfaces[i].name, interfaces[i].ip);

        // send broadcast
        if (sendto(sock, (const void*)&ping, sizeof(pingPack), 0, (struct sockaddr *)&broadcast_addr, addrlen) < 0)
        {
            printf("Failed\n");
        }
    }
    printf("\n");
}

void getResponses(int sock, struct server servers[])
{
    int numServers = 0;
    bool waiting = true;


    int recvlen;
    struct sockaddr_in in_addr;
    socklen_t addrlen = sizeof(in_addr);

    struct pingPack buf;


    // go through the queue of recieved messages if there are any
    printf("Going through sock queue...\n");
    while (waiting)
    {
        recvlen = recvfrom(sock, &buf, sizeof(pingPack), 0, (struct sockaddr *)&in_addr, &addrlen);


        // check if we got anything, if not then probably a timeout
        if (recvlen > 0)
        {
            //printf("%s, %s, %d, %ld, %ld\n", buf.key, buf.name, buf.protocol, buf.time.tv_sec, buf.time.tv_usec);
            char *ip = inet_ntoa(in_addr.sin_addr);

            // validate that this is a server
            if(strcmp(buf.key, SUPERSECRETKEY_SERVER) == 0)
            {
                printf("IP of %s \"%-10s\"", buf.name, ip);

                if (numServers != MAXSERVERS)
                {
                    // add ip to a new server, or to an
                    // old server with same name
                    bool newServer = true;
                    int index = numServers;
                    for (int j = 0; j < numServers; j++)
                    {
                        if (strcmp(servers[j].name, buf.name) == 0)
                        {
                            newServer = false;
                            index = j;
                        }

                    }

                    // add server info
                    strcpy(servers[index].routes[servers[index].numRoutes], ip);
                    memcpy(&servers[index].about, &buf.data, sizeof(struct infoStruct));
                    servers[index].numRoutes++;


                    if (strcmp(ip, lo) == 0)
                    {
                        servers[index].hasLo = true;
                        servers[index].loIndex = index;
                        printf("\tLO");
                    }

                    if (newServer)
                    {
                        strcpy(servers[index].name, buf.name);
                        numServers++;
                    }
                }

                printf("\n");

                //printf("Key: %s, Name: %s\n", serverKey, name);
            }
            else
            {
                printf("Rando found %s\n", ip);
            }
        }
        else
        {
            printf("Finished\n\n");
            waiting = false;
        }
    }

}


void printServerList(struct server *list)
{

    for (int j = 0; j < MAXSERVERS; j = j + 1)
    {
        if (strcmp(list[j].name, "") != 0)
        {
            printf("For server %s\n", list[j].name);
            //printf("%d  %d\n", servers[j].hasLo, servers[j].loIndex);
            for (int q = 0; q < list[j].numRoutes; q++)
            {
                printf("\tFound route \"%s\"", (list[j].routes[q]));
                if (list[j].hasLo && q == list[j].loIndex)
                {
                    printf("\tLO");
                }
                printf("\n");
            }
        }
    }
}

int makeBroadcastSocket()
{
    int bcast_sock;
    struct timeval tv;

    // create udp socket
    bcast_sock = socket(AF_INET, SOCK_DGRAM, 0);


    // create timeout for socket
    tv.tv_sec = DELAY_SECS;
    tv.tv_usec = DELAY_USECS;
    if (setsockopt(bcast_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("opt fail");
    }

    // enable broadcast for socket
    int broadcastEnable = 1;
    int ret = setsockopt(bcast_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    return bcast_sock;
}

void getAllServers(struct server servers[])
{
    printf("Getting servers...\n");
    char hostname[128];

    struct ifa interfaces[10];
    int numFaces = 0;

    int bcast_sock;

    // make the struct start with 0, and servername is empty str
    for (int i = 0; i < MAXSERVERS; i++)
    {
        servers[i].numRoutes = 0;
        servers[i].hasLo = false;
        strcpy(servers[i].name, "");
    }

    // magic for loop
    /**/
    for (int i = 0; i < 0; i++)
    {
        //printf("name %s routes %d lo %d\n\n", servers[i].name, servers[i].numRoutes, servers[i].hasLo);
    }
    /**/

    gethostname(hostname, 128);

    getInterfaces(interfaces, &numFaces);

    bcast_sock = makeBroadcastSocket();

    broadcastAllInterfaces(bcast_sock, interfaces, numFaces, hostname);

    getResponses(bcast_sock, servers);


    /*
    for (int j = 0; j < MAXSERVERS; j = j + 1)
    {
        if (strcmp(servers[j].name, "") != 0)
        {
            (*numServers)++;
            printf("For server %s\n", servers[j].name);
            //printf("%d  %d\n", servers[j].hasLo, servers[j].loIndex);
            for (int q = 0; q < servers[j].numRoutes; q++)
            {
                printf("\tFound route \"%s\"", (servers[j].routes[q]));
                if (servers[j].hasLo && q == servers[j].loIndex)
                {
                    printf("\tLO");
                }
                printf("\n");
            }
        }
    }
    */

    close(bcast_sock);
}

void makeServerListWindow(struct server *theList)
{
    ImGui::NewFrame();

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Lan View", NULL, window_flags);
    ImGui::Text("All servers:");
    ImGui::Text("");



    for (int j = 0; j < MAXSERVERS; j = j + 1)
    {
        if (strcmp(theList[j].name, "") != 0)
        {
            ImGui::Text("Server %s\n", theList[j].name);
            //printf("%d  %d\n", servers[j].hasLo, servers[j].loIndex);
            for (int q = 0; q < theList[j].numRoutes; q++)
            {
                char txt[100];

                if (theList[j].hasLo && q == theList[j].loIndex)
                {
                    sprintf(txt, "%-3s IP %-15s", "LO", (theList[j].routes[q]));
                }
                else
                {
                    sprintf(txt, "%-3s IP %-15s", "", (theList[j].routes[q]));
                }

                sprintf(txt, "%s %-25s %-25s %d/%d %s", txt, theList[j].about.mapName, theList[j].about.gameType, theList[j].about.curPlayers, theList[j].about.maxPlayers, theList[j].about.isCustom ? "C" : "N");

                if (ImGui::Button(txt))
                {
                    printf("Server %s, IP %s\n", theList[j].name, theList[j].routes[q]);
                    if (!connectTo(theList[j].routes[q]))
                    {
                        printf("Failed to connect to: %s at %s\n", theList[j].name, theList[j].routes[q]);
                    }
                    else
                    {
                        connected = true;
                    }
                }
            }
        }
    }



    ImGui::Text("");


    if (ImGui::Button("Update"))
    {
        printf("Loading network\n");

        getAllServers(theList);
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

}
