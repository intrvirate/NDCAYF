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
#include <string>
#include <sys/time.h>
#include <glm/glm.hpp>

#include "networkConfig.hpp"
#include "client.hpp"

int actualSock;
struct sockaddr_in serverAddr;
socklen_t serverAddrLen = sizeof(serverAddr);

struct generalPack buf;
size_t bufSize = sizeof(struct generalPack);

struct sockaddr_in rec_addr;
socklen_t inAddrLen = sizeof(rec_addr);

char hostname[128];
int DELAY_SECS2 = 0;
int DELAY_USECS2 = 100;

int clientID;

struct Unvalid *unvalidated = new struct Unvalid[100];
int numUnvalid = 0;
int unvalidStart = 0;
unsigned int unvalidID = 0;

struct generalPack movePointData = makeBasicPack(MOVE);

bool connected = false;

void resetMove()
{
    unvalidID = 0;
}

void setMove(unsigned int id)
{
    unvalidID = id;
}
void setConnection(bool value)
{
    connected = value;
}

bool getConnection()
{
    return connected;
}

struct sockaddr_in getServerAddr()
{
    return serverAddr;
}

void setServerAddr(struct sockaddr_in newServerAddr)
{
    serverAddr = newServerAddr;
}

int getID()
{
    return clientID;
}

int makeSocket()
{
    struct timeval tv;
    struct sockaddr_in myaddr;

    // create udp socket
    actualSock = socket(AF_INET, SOCK_DGRAM, 0);


    // create timeout for socket
    tv.tv_sec = DELAY_SECS2;
    tv.tv_usec = DELAY_USECS2;

    if (setsockopt(actualSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("opt fail");
        return -1;
    }


    //int broadcastEnable = 1;
    //int ret = setsockopt(actualSock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    return 0;
}


bool connectTo(char ip[])
{
    bool success = false;

    // set the server address to the ip
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    // set the hostname
    gethostname(hostname, 128);

    //make the connect pack, and receve pack
    struct generalPack connectPack = makeBasicPack(CONNECT);
    struct generalPack *msgPack = new struct generalPack;

    //makes the socket, doesn't work it you don't have a socket, as you can imagine
    if (makeSocket() < 0)
    {
        printf("Failed to get socket");
        success = -1;
    }

    // send the connect pack
    if (send(&connectPack) < 0)
    {
        printf("Send failed\n");
    }

    // try to recieve a connect pack back
    bool trying = true;
    bool found = false;
    int tries = 0;
    while (trying)
    {
        if (checkServer(msgPack) < 0)
        {
            tries++;
            perror("Received failed\n");
        }
        else
        {
            found = true;
            trying = false;
        }

        if (tries > 10)
        {
            trying = false;
        }
    }

    printf("protocol %d == %d\n", msgPack->protocol, CONNECT);
    printf("found %s\n", found ? "true" : "false");
    // if found and is a connect packet then we are good
    if (found && (msgPack->protocol == CONNECT))
    {
        success = true;
        printf("%s, %s, %d, %ld, %ld", msgPack->key, msgPack->name, msgPack->protocol, msgPack->time.tv_sec, msgPack->time.tv_usec, msgPack->data);
        // get the int out of the extra bytes
        clientID = (int)*(msgPack->data);
        printf(", %d\n", clientID);
    }
    else
    {
        printf("Failed\n");
        success = false;
    }

    return success;
}

struct generalPack makeBasicPack(int ptl)
{
    struct generalPack pack;
    strcpy(pack.key, SUPERSECRETKEY_CLIENT);
    strcpy(pack.name, hostname);
    pack.protocol = ptl;
    pack.numObjects = 1;

    return pack;
}



// sends to a specified addr
int sendTo(struct generalPack *toSend, struct sockaddr_in *toAddr)
{
    int success = 1;

    //send timestamp, incase you forgot
    gettimeofday(&toSend->time, NULL);

    if (sendto(actualSock, (const void*)toSend, bufSize, 0, (struct sockaddr *)toAddr, serverAddrLen) < 0)
    {
        perror("Failed to send\n");
        success = -1;
    }

    return success;
}

// sends to server
int send(struct generalPack *toSend)
{
    return sendTo(toSend, &serverAddr);
}



// gets from a specified addr
int getFrom(struct generalPack *msg, struct sockaddr_in fromAddr)
{
    int recvlen = -1;
    int success = -1;

    recvlen = recvfrom(actualSock, &buf, bufSize, 0, (struct sockaddr *)&rec_addr, &inAddrLen);
    if (recvlen > 0)
    {
        printf("got a msg\n");
    }

    if (rec_addr.sin_addr.s_addr == fromAddr.sin_addr.s_addr && recvlen > 0)
    {
        success = 0;
        *msg = buf;
        //printf("Received %d bytes\n", recvlen);
        //printf("%s, %s, %d, %ld, %ld\n", msg->key, msg->name, msg->protocol, msg->time.tv_sec, msg->time.tv_usec);
    }

    return success;
}

// gets from server
int checkServer(struct generalPack *msg)
{
    return getFrom(msg, serverAddr);
}


void reconcileClient(struct entities *me, struct move *server, glm::vec3 *cPos)
{
    bool found = false;
    int end = unvalidStart;           // end is the first id greater than the servers id, if one is found
    int actualIndex = unvalidStart;
    int toRmFromArr = 0; // how much can be cut out of the buffer

    for (int i = 0; i < numUnvalid; i++)
    {
        if ((unvalidated[actualIndex].id > me->moveID) && !found)
        {
            toRmFromArr = i;
            printf("%u > %u : us > server\n", unvalidated[actualIndex].id, me->moveID);
            end = actualIndex;
            found = true;
        }

        // increment actual, then check if it when over to loop back
        actualIndex++;
        if (actualIndex > 99)
        {
            actualIndex = 0;
        }
    }

    if ((toRmFromArr == 0) && found) { printf("oh no!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); } // this means the buffer is bad

    // if so then check if we agree at were i was at the servers id
    if (found)
    {
        int newLen = numUnvalid - toRmFromArr;
        // the index of the serverid in unvalidated
        int serverID = end - 1;
        if (end == 0)
        {
            serverID = 99;
        }


        printf("enad %d ", end); printf("curlen %u ", numUnvalid); printf("serverID %d\n", serverID);

        // where the server thinks we are
        printf("server %u == cur %u\n", me->moveID, serverID);

        // these two can be equal, unless lots of lag then probably not
        printf("recent %.2f, %.2f, %.2f\n", unvalidated[unvalidStart + numUnvalid - 1].theMove.pos.x, unvalidated[unvalidStart + numUnvalid - 1].theMove.pos.y, unvalidated[unvalidStart + numUnvalid - 1].theMove.pos.z);
        printf("us %.2f, %.2f, %.2f\n", unvalidated[end].theMove.pos.x, unvalidated[end].theMove.pos.y, unvalidated[end].theMove.pos.z);

        // should be equal unless the server made a change to our pos internally, or it skiped an move because it was bad
        printf("same as server %.2f, %.2f, %.2f\n", unvalidated[serverID].theMove.pos.x, unvalidated[serverID].theMove.pos.y, unvalidated[serverID].theMove.pos.z);
        printf("server %.2f, %.2f, %.2f\n", server->pos.x, server->pos.y, server->pos.z);

        // check if our pos (with the same id as the server) is equal to the servers pos
        if (unvalidated[serverID].theMove.pos.x == server->pos.x && unvalidated[serverID].theMove.pos.y == server->pos.y && unvalidated[serverID].theMove.pos.z == server->pos.z)
        {
            printf("doing nothing\n");
        }
        else
        {
            // they are off so we apply the difference of the points to our pos
            glm::vec3 diff = unvalidated[serverID].theMove.pos - server->pos;
            printf("the DIFF %.2f, %.2f, %.2f====================================================================\n", diff.x, diff.y, diff.z);
            *cPos += diff;

            // make the last unvalid point reflect the change
            unvalidated[unvalidStart + numUnvalid - 1].theMove.pos = *cPos;
        }


        // the buffer is 
        unvalidStart = end;
        numUnvalid = newLen;
    }
    else
    {
        // move start to after these points, make sure it wraps
        unvalidStart += numUnvalid;
        if (unvalidStart > 99)
        {
            unvalidStart = unvalidStart - 100;
        }

        numUnvalid = 0;

        *cPos = server->pos;
    }
}

void netLog(glm::vec3 pos, glm::vec3 front, char key[])
{
    struct move moveData;

    moveData.pos = pos;
    moveData.dir = front;
    strcpy(moveData.extraActions, key);

    // gets the next position in buf, and wraps it
    int nextValue = unvalidStart + numUnvalid;
    if (nextValue > 99)
    {
        nextValue = nextValue - 100;
    }

    unvalidated[nextValue].theMove = moveData;
    unvalidated[nextValue].id = unvalidID;


    // copy to the packet we send
    memcpy(&movePointData.data, &moveData, sizeof(moveData));
    memcpy(&movePointData.data[sizeof(moveData)], &unvalidID, sizeof(unvalidID));

    // increment the size of buf, and the id
    unvalidID++;
    numUnvalid++;

    send(&movePointData);
}
