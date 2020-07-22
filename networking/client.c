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

#define PORT  12345
#define BUFSIZE 2048
#define SUPERSECRETKEY_SERVER "ndcayfserver"
#define SUPERSECRETKEY_CLIENT "ndcayfclient"

struct ifa {
    char name[128];
    char ip[128];
};

int getInterfaces(struct ifa interfaces[]);
void broadcastAllInterfaces(int sock, struct ifa interfaces[], int elements, char name[]);
void getResponses(int sock);

int getInterfaces(struct ifa interfaces[])
{
    int i = 0;
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

            // adds to the array
            strcpy(interfaces[i].name, ifaddr->ifa_name);
            strcpy(interfaces[i].ip, host);
            i++;

            //printf("Interface: %-20sBroadcast: %s\n", ifaddr->ifa_name, host);

       }

        ifaddr = ifaddr->ifa_next;

    }

    // free up the space, and return the amount of elements in the array
    freeifaddrs(ifaddr);
    return i;
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


        printf("Broadcast to %s\n", interfaces[i].ip);

        // send broadcast
        if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&broadcast_addr, addrlen) < 0)
        {
            printf("Failed\n");
        }
    }

}

void getResponses(int sock)
{
    int i = 0;
    bool waiting = true;
    struct sockaddr_in servers[10];

    struct sockaddr_in in_addr;
    unsigned char buf[BUFSIZE];
    int recvlen;
    socklen_t addrlen = sizeof(in_addr);

    char name[128];
    char serverKey[128];


    printf("Going through sock queue...\n");
    while (waiting)
    {
        recvlen = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&in_addr, &addrlen);


        if (recvlen > 0)
        {
            buf[recvlen] = 0;

            // add to server list the ip
            servers[i] = in_addr;
            strcpy(serverKey, strtok(buf, "$"));
            char *ip = inet_ntoa(in_addr.sin_addr);
            //printf("%s == %s\n", serverKey, SUPERSECRETKEY);

            if (strcmp(serverKey, SUPERSECRETKEY_CLIENT) == 0)
            {
                strcpy(name, strtok(NULL, "$"));
                printf("Key: %s, Name: %s\n", serverKey, name);
                printf("IP of %s \"%-10s\"\n", buf, ip);
                i++;
            }
            else
            {
                printf("Rando found %s\n", ip);
            }
        }
        else
        {
            printf("Nothing received\n");
            waiting = false;
        }
    }

    for (int j = 0; j < i; j = j + 1)
    {
        char *ip = inet_ntoa(servers[j].sin_addr);
        printf("Captured \"%s\"\n", ip);
    }

}


int main(void)
{
    char hostname[128];

    struct ifa interfaces[10];
    int numFaces;

    struct timeval tv;
    int bcast_sock;

    numFaces = getInterfaces(interfaces);

    gethostname(hostname, 128);

    /*
    for (int jj = 0; jj < numFaces; jj++)
    {
        printf("Name %-20s ip %s\n", interfaces[jj].name, interfaces[jj].ip);

    }
    */

    // create udp socket
    bcast_sock = socket(AF_INET, SOCK_DGRAM, 0);

    /* create timeout for socket */
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(bcast_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("opt fail");
    }

    // enable broadcast for socket
    int broadcastEnable = 1;
    int ret = setsockopt(bcast_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    broadcastAllInterfaces(bcast_sock, interfaces, numFaces, hostname);

    getResponses(bcast_sock);

    return 0;
}
