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

#include "networkConfig.c"
#include "getLan.h"

//struct ifa {
//    char name[128];
//    char ip[128];
//};
//
//struct server {
//    struct sockaddr_in routes[5];
//    int numRoutes;
//    char name[128];
//    bool hasLo;
//    int loIndex;
//};

char lo[128];
int DELAY_SECS = 1;
int DELAY_USECS = 0;

//void getInterfaces(struct ifa interfaces[], int *numFaces);
//void broadcastAllInterfaces(int sock, struct ifa interfaces[], int elements, char name[]);
//void getResponses(int sock, struct server servers[]);
//void getAllServers(struct server servers[]);

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

            // adds to the array
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
    sprintf(msg, "%s$%s", SUPERSECRETKEY_CLIENT, name);

    socklen_t addrlen = sizeof(broadcast_addr);        /* length of addresses */

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
        if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&broadcast_addr, addrlen) < 0)
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
    struct sockaddr_in in_addr;
    socklen_t addrlen = sizeof(in_addr);

    char buf[BUFSIZE];
    int recvlen;

    char name[128];
    char serverKey[128];

    // go through the queue of recieved messages if there are any
    printf("Going through sock queue...\n");
    while (waiting)
    {
        recvlen = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&in_addr, &addrlen);


        // check if we got anything, if not then probably a timeout
        if (recvlen > 0)
        {
            buf[recvlen] = 0;

            strcpy(serverKey, strtok(buf, "$"));
            char *ip = inet_ntoa(in_addr.sin_addr);

            // validate that this is a server
            if (strcmp(serverKey, SUPERSECRETKEY_SERVER) == 0)
            {
                strcpy(name, strtok(NULL, "$"));
                printf("IP of %s \"%-10s\"", name, ip);


                if (numServers != MAXSERVERS)
                {
                    // add ip to a new server, or to an
                    // old server with same name
                    bool newServer = true;
                    int index = numServers;
                    for (int j = 0; j < numServers; j++)
                    {
                        if (strcmp(servers[j].name, name) == 0)
                        {
                            newServer = false;
                            index = j;
                        }

                    }

                    // add server info
                    servers[index].routes[servers[index].numRoutes] = in_addr;
                    servers[index].numRoutes++;

                    if (strcmp(ip, lo) == 0)
                    {
                        servers[index].hasLo = true;
                        servers[index].loIndex = index;
                        printf("\tLO");
                    }

                    if (newServer)
                    {
                        strcpy(servers[index].name, name);
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

    /*
    for (int j = 0; j < i; j = j + 1)
    {
        printf("For server %s\n", servers[j].name);
        for (int q = 0; q < servers[j].numRoutes; q++)
        {
            printf("\tFound route \"%s\"\n", inet_ntoa(servers[j].routes[q].sin_addr));
        }
    }
    */

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
    char hostname[128];

    struct ifa interfaces[10];
    int numFaces;

    int bcast_sock;

    // make the struct start with 0, and servername is empty str
    for (int i = 0; i < MAXSERVERS; i++)
    {
        servers[i].numRoutes = 0;
        servers[i].hasLo = false;
        strcpy(servers[i].name, "");
    }
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
                printf("\tFound route \"%s\"", inet_ntoa(servers[j].routes[q].sin_addr));
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


/*
int main(void)
{
    struct server serverList[MAXSERVERS];

    getAllServers(serverList);

    for (int j = 0; j < MAXSERVERS; j = j + 1)
    {
        if (strcmp(serverList[j].name, "") != 0)
        {
            printf("For server %s\n", serverList[j].name);
            //printf("%d  %d\n", servers[j].hasLo, servers[j].loIndex);
            for (int q = 0; q < serverList[j].numRoutes; q++)
            {
                printf("\tFound route \"%s\"", inet_ntoa(serverList[j].routes[q].sin_addr));
                if (serverList[j].hasLo && q == serverList[j].loIndex)
                {
                    printf("\tLO");
                }
                printf("\n");
            }
        }
    }
}
*/
