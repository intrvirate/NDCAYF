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

#include <sys/ioctl.h>
#include <linux/wireless.h>

#include "networkConfig.hpp"
#include "getLan.hpp"
#include "client.hpp"

struct sockaddr_in lo;
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

//https://gist.github.com/edufelipe/6108057
int check_wireless(const char* ifname, char* protocol) {
  int sock = -1;
  struct iwreq pwrq;
  memset(&pwrq, 0, sizeof(pwrq));
  strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 0;
  }

  if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) {
    if (protocol) strncpy(protocol, pwrq.u.name, IFNAMSIZ);
    close(sock);
    return 1;
  }

  close(sock);
  return 0;
}


void getBroadCast(struct sockaddr_in ip, struct in_addr subnet, struct in_addr *broadcastOut)
{
    // the only necessary part
    broadcastOut->s_addr = ip.sin_addr.s_addr | ~subnet.s_addr;

    // for printing
    char broadcast_address[INET_ADDRSTRLEN];

    char bd[128];
    strcpy(bd, inet_ntoa(*broadcastOut));

    char ipp[128];
    strcpy(ipp, inet_ntoa(ip.sin_addr));

    char sb[128];
    strcpy(sb, inet_ntoa(subnet));

    /*
    if (inet_ntop(AF_INET, broadcastOut, broadcast_address, INET_ADDRSTRLEN) != NULL) {
        //printf("Broadcast address of %s with netmask %s is %s\n", ipp, sb, broadcast_address);
    }
    else {
        fprintf(stderr, "Failed converting number to string\n");
    }
    */

    //printf("Broadcast address of %s with netmask %s is %s\n", ipp, sb, bd);

}


void getInterfaces(struct ifa interfaces[], int *numFaces)
{
    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];
    bool isLo;

    getifaddrs(&ifaddr);

    // loops through each element in ifaddr
    while (ifaddr)
    {
        isLo = false;
        family = ifaddr->ifa_addr->sa_family;


        // checks if it is the right kind
        if (family == AF_INET) {

            struct sockaddr_in *ip_raw = (struct sockaddr_in *) ifaddr->ifa_addr;
            struct sockaddr_in *sub_raw = (struct sockaddr_in *) ifaddr->ifa_netmask;

            // get our ip on this interface
            char ip[128];
            strcpy(ip, inet_ntoa(ip_raw->sin_addr));
            interfaces[*numFaces].ip = ip_raw->sin_addr;

            // get subnet
            char subnet[128];
            strcpy(subnet, inet_ntoa(sub_raw->sin_addr));
            interfaces[*numFaces].subnet = sub_raw->sin_addr;

            // checks if lo
            if (strcmp(ifaddr->ifa_name, "lo") == 0) {
                lo = *ip_raw;
                isLo = true;
                interfaces[*numFaces].isLo = true;
                //printf("lo %s %d\n", ip, *numFaces);
            }

            // if lo then don't worry about broadcast
            if (isLo) {
                interfaces[*numFaces].broadcast = ip_raw->sin_addr;

            }
            else {
                getBroadCast(*ip_raw, interfaces[*numFaces].subnet, &interfaces[*numFaces].broadcast);

            }

            char broad[128];
            strcpy(broad, inet_ntoa(interfaces[*numFaces].broadcast));
            /*
            printf("broadcast: %s\n", broad);
            printf("ip: %s\n", inet_ntoa(interfaces[*numFaces].ip));
            printf("subnet: %s\n", inet_ntoa(interfaces[*numFaces].subnet));
            */

            char protocol[IFNAMSIZ]  = {0};
            if (check_wireless(ifaddr->ifa_name, protocol)) {
              interfaces[*numFaces].isWifi = true;
              //printf("interface %s is wireless: %s\n", ifaddr->ifa_name, protocol);
            }
            else {
              interfaces[*numFaces].isWifi = false;
              //printf("interface %s is not wireless\n", ifaddr->ifa_name);
            }


            strcpy(interfaces[*numFaces].name, ifaddr->ifa_name);
            (*numFaces)++;

            printf("Interface: %s\n\tBroadcast: %s\n\tIP: %s\n\tSubnet: %s\n", ifaddr->ifa_name, broad, ip, subnet);

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
        broadcast_addr.sin_addr.s_addr = interfaces[i].broadcast.s_addr;

        printf("Broadcast to %-15s %s\n", interfaces[i].name, inet_ntoa(interfaces[i].broadcast));

        // send broadcast
        if (sendto(sock, (const void*)&ping, sizeof(pingPack), 0, (struct sockaddr *)&broadcast_addr, addrlen) < 0)
        {
            printf("Failed\n");
        }
    }
    printf("\n");
}



void getResponses(int sock, struct server servers[], struct ifa interfaces[], int numFaces)
{
    int numServers = 0;
    bool waiting = true;
    bool hasLo;


    int recvlen;
    struct sockaddr_in rec_addr;
    socklen_t addrlen = sizeof(rec_addr);

    struct pingPack buf;


    // go through the queue of recieved messages if there are any
    printf("Going through sock queue...\n");
    while (waiting)
    {
        recvlen = recvfrom(sock, &buf, sizeof(pingPack), 0, (struct sockaddr *)&rec_addr, &addrlen);
        hasLo = false;


        // check if we got anything, if not then probably a timeout
        if (recvlen > 0)
        {
            //printf("%s, %s, %d, %ld, %ld\n", buf.key, buf.name, buf.protocol, buf.time.tv_sec, buf.time.tv_usec);

            // validate that this is a server
            if(strcmp(buf.key, SUPERSECRETKEY_SERVER) == 0)
            {
                printf("IP of %s \"%-10s\"", buf.name, inet_ntoa(rec_addr.sin_addr));
                //printf("\n ip vs lo %s %s\n", inet_ntoa(rec_addr.sin_addr), inet_ntoa(lo.sin_addr));

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

                    int ifaIndex = -1;
                    // if lo then just find the interface that is lo
                    if (lo.sin_addr.s_addr == rec_addr.sin_addr.s_addr)
                    {
                        servers[index].hasLo = true;
                        servers[index].loIndex = index;
                        printf("\tLO");

                        for (int i = 0; i < numFaces; i++)
                        {
                            // weird bug that makes the next interface be lo too
                            if (interfaces[i].isLo && ifaIndex == -1)
                            {
                                ifaIndex = i;
                                //printf("gt lo %d %s %s\n", i, inet_ntoa(interfaces[i].ip), interfaces[i].isLo ? "t" : "f");
                            }
                        }
                    }
                    else {
                        // make the broadcast using the subnet of each and see which interface has the same broadcast addr
                        struct in_addr broadCast;
                        for (int i = 0; i < numFaces; i++) {
                            getBroadCast(rec_addr, interfaces[i].subnet, &broadCast);
                            if (broadCast.s_addr == interfaces[i].broadcast.s_addr) {
                                //printf("got one\n");
                                ifaIndex = i;

                            }
                        }
                    }

                    if (ifaIndex == -1) {
                        printf("We have an issue");
                    }

                    // add server info
                    servers[index].routes[servers[index].numRoutes] = interfaces[ifaIndex];
                    memcpy(&servers[index].about, &buf.data, sizeof(struct infoStruct));
                    servers[index].numRoutes++;

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
                printf("Rando found %s\n", inet_ntoa(rec_addr.sin_addr));
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
                printf("\tFound route \"%s\"", inet_ntoa(list[j].routes[q].ip));
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

    getResponses(bcast_sock, servers, interfaces, numFaces);


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
    ImGui::Text(" ");



    for (int j = 0; j < MAXSERVERS; j = j + 1)
    {
        if (strcmp(theList[j].name, "") != 0)
        {
            ImGui::Text("Server %s\n", theList[j].name);
            //printf("%d  %d\n", servers[j].hasLo, servers[j].loIndex);
            for (int q = 0; q < theList[j].numRoutes; q++)
            {
                char txt[100];
                char matchType[2];
                char connectionType[10];

                char *ip = inet_ntoa(theList[j].routes[q].ip);

                if (theList[j].hasLo && q == theList[j].loIndex)
                {
                    sprintf(txt, "%-3s IP %-15s", "LO", ip);
                    strcpy(connectionType, "LO   ");
                }
                else
                {
                    sprintf(txt, "%-3s IP %-15s", "", ip);

                    if (theList[j].routes[q].isWifi) {
                        strcpy(connectionType, "Wifi ");
                    }
                    else {
                        strcpy(connectionType, "Ether");
                    }
                }

                if (theList[j].about.isCustom) {
                    strcpy(matchType, "C");
                }
                else {
                    strcpy(matchType, "N");
                }

                sprintf(txt, "%s %-25s %-25s %d/%d %s %s", txt, theList[j].about.mapName, theList[j].about.gameType, theList[j].about.curPlayers, theList[j].about.maxPlayers, matchType, connectionType);

                if (ImGui::Button(txt))
                {
                    printf("Server %s, IP %s\n", theList[j].name, ip);
                    if (!connectTo(ip))
                    {
                        printf("Failed to connect to: %s at %s\n", theList[j].name, ip);
                    }
                    else
                    {
                        setConnection(true);
                    }
                }
            }
        }
    }



    ImGui::Text(" ");


    if (ImGui::Button("Update"))
    {
        printf("Loading network\n");

        getAllServers(theList);
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

}
