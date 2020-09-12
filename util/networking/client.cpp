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

int keyID = 0;
int clientID;
//struct datapoint *unvalidated = new struct datapoint[10000];
int unvalidSize = 0;
bool connected = false;
bool test_nw_cl = false;

const glm::vec3 upp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeedp = 0.05f;

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

int getKeyID()
{
    return keyID;
}

void setTestNw(bool value)
{
    test_nw_cl = value;
}

// yes i did copy the source code of printf
int debugPrint(const char *format, ...)
{
   int done = 0;
   if (test_nw_cl)
   {
       va_list arg;

       va_start (arg, format);
       done = vfprintf (stdout, format, arg);
       va_end (arg);
   }

   return done;
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
    if (send(connectPack) < 0)
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


int send(struct generalPack toSend)
{
    int success = 1;

    //send timestamp, incase you forgot
    gettimeofday(&toSend.time, NULL);

    if (sendto(actualSock, (const void*)&toSend, bufSize, 0, (struct sockaddr *)&serverAddr, serverAddrLen) < 0)
    {
        perror("Failed to send\n");
        success = -1;
    }

    return success;
}

int sendTo(struct generalPack toSend, struct sockaddr_in toAddr)
{
    int success = 1;

    //send timestamp, incase you forgot
    gettimeofday(&toSend.time, NULL);

    if (sendto(actualSock, (const void*)&toSend, bufSize, 0, (struct sockaddr *)&toAddr, serverAddrLen) < 0)
    {
        perror("Failed to send\n");
        success = -1;
    }

    return success;
}

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

int checkServer(struct generalPack *msg)
{
    int recvlen = -1;
    int success = -1;

    recvlen = recvfrom(actualSock, &buf, bufSize, 0, (struct sockaddr *)&rec_addr, &inAddrLen);
    if (recvlen > 0)
    {
        printf("got a msg\n");
    }

    if (rec_addr.sin_addr.s_addr == serverAddr.sin_addr.s_addr && recvlen > 0)
    {
        success = 0;
        *msg = buf;
        //printf("Received %d bytes\n", recvlen);
        //printf("%s, %s, %d, %ld, %ld\n", msg->key, msg->name, msg->protocol, msg->time.tv_sec, msg->time.tv_usec);
    }

    return success;
}


void getParts(std::string parts[], std::string raw, int amount, std::string deli)
{
    size_t pos = 0;
    int cur = 0;
    std::string token;
    while ((pos = raw.find(deli)) != std::string::npos) {
        //debugPrint("cur %d\n", cur);
        token = raw.substr(0, pos);
        parts[cur] = token;
        cur++;
        raw.erase(0, pos + deli.length());
    }
    parts[cur] = raw;
}


void applyDumpData(struct entities *them, char data[], int *count)
{
    /*
    ///////remove/////////////
    //clientID = 0;
    ////////////////////////
    *count = 0;

    std::string raw = data;
    // one for each player
    std::string parts[MAXPLAYERS];
    std::string playerdeli("(");

    // movesments per dump * 2, at one second intervals thats 60 + 60, 
    // though sometimes i get more than that, 132
    int numberOfMoves = 164;
    std::string section[numberOfMoves];
    std::string secdeli("&");

    // store pos, float
    std::string floats[3];
    std::string fldeli(",");
    // which entity we are on
    int id;


    debugPrint("beforeLoop\n");
    getParts(parts, raw, 10, playerdeli);
    for (int i = 0; i < MAXPLAYERS; i++)
    {
        // for each player/entity
        if (strcmp(parts[i].c_str(), "") != 0)
        {
            debugPrint("loop number %d\n", i);
            debugPrint("%s\n", parts[i].c_str());
            (*count)++;
            //printf("%s\n", parts[i].c_str());
            // get each section
            getParts(section, parts[i], 60, secdeli);
            debugPrint("got sections\n");

            id = std::stoi(section[0].c_str());


            // get pos
            getParts(floats, section[1], 3, fldeli);
            debugPrint("got pos\n");
            them[id].cameraPos = glm::vec3(std::stod(floats[0]), std::stod(floats[1]), std::stod(floats[2]));

            // get dir
            getParts(floats, section[2], 3, fldeli);
            debugPrint("got dir\n");
            them[id].cameraDirection = glm::vec3(std::stod(floats[0]), std::stod(floats[1]), std::stod(floats[2]));

            if (id == clientID)
            {
                // last value is the keyid, apply all key ids greater than this one, for reconciliation
                them[id].moveID = std::stoi(section[3]);

            }
            else
            {
                // all remaining sections are of the format
                // f,f,f s
                // but we want both a f,f,f and a s
                // ex 10.0,10.0,10.0&was&0.0,0.0,0.0,&w
                them[id].numMoves = 0;
                debugPrint("nextLoop\n");
                for (int j = 3; j < numberOfMoves; j = j + 2)
                {
                    struct move action;
                    if (strcmp(section[j].c_str(), "") != 0 && section[j].length() > 15)
                    {
                        debugPrint("number %d, data %s\n", j, section[j].c_str());
                        //get the dir

                        getParts(floats, section[j], 3, fldeli);
                        action.dir = glm::vec3(std::stod(floats[0]), std::stod(floats[1]), std::stod(floats[2]));

                        // get the moves
                        strcpy(action.moves, section[j + 1].c_str());

                        them[id].keys[them[id].numMoves] = action;
                        them[id].numMoves++;

                    }
                }
            }
        }
    }
    */
}

void reconcileClient(struct entities *me)
{
    /*
    //printf("im at %d servers at %d size %d\n", unvalidated[unvalidSize - 1].id, me->moveID, unvalidSize);

    int end;
    int newLen;
    bool found = false;
    glm::vec3 newPos = me->cameraPos;
    glm::vec3 fakeDir;

    if (unvalidSize == 0)
    {
        //printf("Early\n");
        return ;
    }

    for (int i = 0; i < unvalidSize; i++)
    {
        if (unvalidated[i].id > me->moveID && !found)
        {
            end = i;
            found = true;
        }
    }

    if (found)
    {
        newLen = unvalidSize - end;

        printf("Applying %d moves to client\n", newLen);

        // makes new arr, with the unvalidated points from the old, and space for the new
        struct datapoint *newUnvalid = new struct datapoint[newLen + 1000];


        // copy to the new list
        for (int i = 0; i < newLen; i++)
        {
            newUnvalid[i].id = unvalidated[i + end].id;
            strcpy(newUnvalid[i].move, unvalidated[i + end].move);
            strcpy(newUnvalid[i].direction, unvalidated[i + end].direction);

        }


        std::string raw;
        std::string floats[3];
        std::string fldeli(",");
        // move
        for (int i = 0; i < newLen; i++)
        {
            printf("\tMoving %s, dir %s, id %d\n", newUnvalid[i].move, newUnvalid[i].direction, newUnvalid[i].id);
            // get direction
            raw = newUnvalid[i].direction;
            getParts(floats, raw, 3, fldeli);

            fakeDir = glm::vec3(std::stod(floats[0]), std::stod(floats[1]), std::stod(floats[2]));

            // move position
                        //printf("before [%.3f,%.3f,%.3f]\n", newPos.x, newPos.y, newPos.z);
            applyKeys(newUnvalid[i].move, fakeDir, &newPos);

                        //printf("after [%.3f,%.3f,%.3f]\n", newPos.x, newPos.y, newPos.z);
        }

        // set to the new pos
        me->cameraPos = newPos;

        // remove old list replace with the new one
        delete [] unvalidated;
        unvalidated = newUnvalid;
        unvalidSize = newLen;
    }
    else
    {
        // we are on insync, make a new invalid list
        //printf("newList\n");
        delete [] unvalidated;
        unvalidated = new struct datapoint[1000];
        unvalidSize = 0;

    }
    */



}

void applyKeys(char keys[], glm::vec3 dir, glm::vec3 *pos)
{
    // apply for each key
    glm::vec3 cameraRight = glm::normalize(glm::cross(upp, dir));
    glm::vec3 cameraUp = glm::cross(dir, cameraRight);

    for (int i = 0; i < strlen(keys); i++)
    {

        if (keys[i] == *UNI_FD)
            *pos += cameraSpeedp * dir;

        if (keys[i] == *UNI_BK)
            *pos -= cameraSpeedp * dir;

        if (keys[i] == *UNI_LT)
            *pos -= glm::normalize(glm::cross(dir, cameraUp)) * cameraSpeedp;

        if (keys[i] == *UNI_RT)
            *pos += glm::normalize(glm::cross(dir, cameraUp)) * cameraSpeedp;
    }
}


void netLog(glm::vec3 pos, glm::vec3 front, char key[])
{
    // separates each key by , useless
    /*
    int len = key.length();
    for (int i = 0; i < len; i++)
    {
        key.insert(i*2 + 1, ",");
    }
    key.pop_back();

    //key.insert(0, "(");
    //key.append(")");
    */

    //strcpy(unvalidated[unvalidSize].move, key.c_str());

    //strcpy(unvalidated[unvalidSize].direction, dir);

    //unvalidated[unvalidSize].id = keyID++;

    //printf("Key: [%s], dir: [%s], [%d]\n", unvalidated[unvalidSize].move, unvalidated[unvalidSize].direction, unvalidated[unvalidSize].id);
    struct generalPack movePoint = makeBasicPack(MOVE);
    struct move moveData;

    moveData.pos = pos;
    moveData.dir = front;
    strcpy(moveData.extraActions, key);
    unsigned int moveID = 10;

    //char * ptr = *((char)moveData + (char)a);


    /* debug
    printf("[");
    for (int i = 0; i < sizeof(moveID); i++)
    {
        printf("%02x", ((char *)&moveID)[i]);
    }
    printf("]\n");


    printf("[");
    for (int i = 0; i < sizeof(moveData); i++)
    {
        printf("%02x", ((char*)&moveData)[i]);
    }
    printf("]\n");
    */

    // copy to the packet buffer
    memcpy(&movePoint.data, &moveData, sizeof(moveData));
    memcpy(&movePoint.data[sizeof(moveData)], &moveID, sizeof(moveID));

    /* debug
    printf("[");
    for (int i = 0; i < 1040; i++)
    {
        printf("%02x", ((char*)&movePoint)[i]);
    }
    printf("]\n");

    char * ptr = (char*)&movePoint;
    struct generalPack pack = *(struct generalPack*)ptr;

    struct move moveafter;
    memcpy(&moveafter, &pack.data, sizeof(struct move));

    int aa;
    memcpy(&aa, &pack.data[sizeof(struct move)], sizeof(int));

    printf("%s\n", moveafter.extraActions);
    printf("%d\n", aa);

    printf("%d\n", sizeof(movePoint));
    // debug*/

    send(movePoint);
    /*
    if (sendMoveData(unvalidated[unvalidSize]) < 0)
    {
        printf("Something went wrong with key send\n");
    }
    unvalidSize++;
    */
}
