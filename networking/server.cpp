#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "networkConfig.h"

int main(int argc, char **argv)
{
    struct sockaddr_in myaddr;    /* our address */
    struct sockaddr_in ether;
    struct sockaddr_in remaddr;    /* remote address */
    socklen_t addrlen = sizeof(remaddr);        /* length of addresses */
    int recvlen;            /* # bytes received */
    int fd;                /* our socket */
    int other;
    unsigned char buf[BUFSIZE];    /* receive buffer */
    char reply[128];
    char hostname[128]; /* host name, for debugging */

    gethostname(hostname, 128);

    sprintf(reply, "%s$%s", SUPERSECRETKEY_SERVER, hostname);

    /* create a UDP socket */

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        return 0;
    }

    /* bind the socket to any valid IP address and a specific port */

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(PORT);

    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }

    /* now loop, receiving data and printing what we received */
    for (;;) {
        printf("waiting on port %d\n", PORT);
        recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
        printf("received %d bytes\n", recvlen);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            printf("received message: \"%s\"\n", buf);


            if (sendto(fd, reply, strlen(reply), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
            {
                printf("Failed to send back\n");
            }
        }
    }
    /* never exits */
}
