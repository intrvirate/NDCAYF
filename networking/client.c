#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT  12345
#define BUFSIZE 2048

int main(void)
{
    struct sockaddr_in broadcast_addr;
    int recvlen;            /* # bytes received */
    socklen_t addrlen = sizeof(broadcast_addr);        /* length of addresses */
    unsigned char buf[BUFSIZE];    /* receive buffer */
    /* create a socket */

    int bcast_sock = socket(AF_INET, SOCK_DGRAM, 0);

    int broadcastEnable = 1;
    int ret = setsockopt(bcast_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    char msg[] = "hi";

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    printf("Broadcasting msg\n");
    if (sendto(bcast_sock, msg, strlen(msg), 0, (struct sockaddr *)&broadcast_addr, addrlen) < 0)
    {
        printf("Failed\n");
    }

    printf("recieving\n");
    recvlen = recvfrom(bcast_sock, buf, BUFSIZE, 0, (struct sockaddr *)&broadcast_addr, &addrlen);
    printf("Ip of server \"%s\"\n", inet_ntoa(broadcast_addr.sin_addr));
    //printf("error %s\n", h_errno);
    if (recvlen > 0)
    {
        buf[recvlen] = 0;
        printf("Recieved \"%s\"\n", buf);
    }

    return 0;
}
