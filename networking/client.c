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

#include "networkConfig.h"
#include "getLan.h"

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
