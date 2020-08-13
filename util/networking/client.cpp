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

char hostname[128];
int DELAY_SECS2 = 0;
int DELAY_USECS2 = 100;

int keyID = 0;
int clientID;
struct datapoint *unvalidated = new struct datapoint[10000];
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

    return 0;
}

unsigned long long getMilliSeconds()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return ((unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000);
}


void composeMsg(char msg[], int protocol, char extra[])
{
    unsigned long long milliSeconds = getMilliSeconds();

    sprintf(msg, "%s$%s$%d$%llu$%d$%s", SUPERSECRETKEY_CLIENT, hostname, protocol, milliSeconds, clientID, extra);
}

int makePacket(char msg[], struct MsgPacket *out)
{
    int state = 0;
    char key[128];
    char extra[256];

    strcpy(key, strtok(msg, "$"));

    if (strcmp(key, SUPERSECRETKEY_SERVER) == 0)
    {
        strcpy(out->name, strtok(NULL, "$"));
        out->ptl = std::stoi(strtok(NULL, "$"));
        out->time = std::stoull(strtok(NULL, "$"));
        strcpy(out->data, strtok(NULL, "$"));
        strcpy(extra, out->data);
        clientID = std::stoi(strtok(extra, "&"));
    }
    else
    {
        printf("Key check failed\n");
        state = -1;
    }
    return state;
}

int connectToServer(char ip[], struct MsgPacket *msg)
{
    gethostname(hostname, 128);
    int success = 1;

    struct sockaddr_in in_addr;
    socklen_t addrlen = sizeof(in_addr);
    char buf[BUFSIZE*2];
    int recvlen = -1;

    socklen_t addrlen_in = sizeof(serverAddr);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    char connectMsg[BUFSIZE];

    composeMsg(connectMsg, CONNECT);

    if (makeSocket() < 0)
    {
        printf("Failed to get socket");
        success = -1;
    }


    if (sendto(actualSock, connectMsg, strlen(connectMsg), 0, (struct sockaddr *)&serverAddr, addrlen_in) < 0)
    {
        printf("Failed to send\n");
        success = -1;
    }

    // actually get the response
    int tries = 0;
    while (recvlen < 0)
    {
        recvlen = recvfrom(actualSock, buf, BUFSIZE, 0, (struct sockaddr *)&in_addr, &addrlen);
        tries++;
        if (tries > 1000)
        {
            success = -1;
            printf("Failed to connect server\n");
        }
    }

    if (!(success < 0))
    {
        printf("Recieved: %s\n", buf);

        if (makePacket(buf, msg) < 0)
        {
            printf("Couldn't turn into packet\n");
            success = -1;
        }
    }

    return success;
}

int sendMoveData(struct datapoint point)
{
    char data[2048];
    char msg[BUFSIZE];
    int success = 1;

    sprintf(data, "%s&%s&%d", point.direction, point.move, point.id);
    socklen_t addrlen_in = sizeof(serverAddr);


    composeMsg(msg, MOVE, data);
    //printf("Sending: %s\n", msg);


    if (sendto(actualSock, msg, strlen(msg), 0, (struct sockaddr *)&serverAddr, addrlen_in) < 0)
    {
        printf("Failed to send\n");
        success = -1;
    }

    return success;
}

int checkServer(char buf[])
{
    struct sockaddr_in in_addr;
    socklen_t addrlen = sizeof(in_addr);
    char in[BUFSIZE*2];
    int recvlen = -1;

    recvlen = recvfrom(actualSock, in, (BUFSIZE*2), 0, (struct sockaddr *)&in_addr, &addrlen);

    if (in_addr.sin_addr.s_addr == serverAddr.sin_addr.s_addr)
    {
        strcpy(buf, in);
        printf("buffer [%s]\n", buf);

    }

    return recvlen;
}


int processMsg(char msg[], struct MsgPacket *packet)
{
    char clientKey[128];
    char name[128];
    char protocol[128];
    char data[BUFSIZE];
    int ptl;
    char time[256];
    strcpy(clientKey, strtok(msg, "$"));


    if (strcmp(clientKey, SUPERSECRETKEY_SERVER) == 0)
    {
        strcpy(name, strtok(NULL, "$"));

        // get protocol and make it a number
        strcpy(protocol, strtok(NULL, "$"));
        ptl = std::stoi(protocol);

        strcpy(time, strtok(NULL, "$"));

        // safe guard
        if (ptl == PING)
        {
            strcpy(data, " ");;
        }
        else
        {
            strcpy(data, strtok(NULL, "$"));
        }


        strcpy(packet->name, name);
        strcpy(packet->data, data);
        packet->ptl = ptl;
        packet->time = atoll(time);

        if (ptl == PING)
        {
            printf("Send response\n");
            return PONG;
        }
        else if (ptl ==  CONNECT)
        {
            printf("\"Connecting\" client\n");
            return CONNECT;
        }
        else if (ptl == DUMP)
        {
            //printf("Got a server dump\n");
            return DUMP;
        }
    }
    else
    {
        printf("Not a NDCAYF client\n");
        return -1;
    }
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
    std::string section[164];
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
                //reconcileClient(them[id], std::stoi(section[3]);
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
                for (int j = 3; j < 31; j = j + 2)
                {
                    struct move action;
                    if (strcmp(section[j].c_str(), "") != 0)
                    {
                        debugPrint("number %d\n", j);
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
}

void reconcileClient(struct entities *me)
{
    printf("im at %d servers at %d size %d\n", unvalidated[unvalidSize - 1].id, me->moveID, unvalidSize);

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


void netLog(std::string key, glm::vec3 front)
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

    strcpy(unvalidated[unvalidSize].move, key.c_str());

    char dir[30];
    sprintf(dir, "%.3f,%.3f,%.3f", front.x, front.y, front.z);

    strcpy(unvalidated[unvalidSize].direction, dir);

    unvalidated[unvalidSize].id = keyID++;

    //printf("Key: [%s], dir: [%s], [%d]\n", unvalidated[unvalidSize].move, unvalidated[unvalidSize].direction, unvalidated[unvalidSize].id);

    if (sendMoveData(unvalidated[unvalidSize]) < 0)
    {
        printf("Something went wrong with key send\n");
    }
    unvalidSize++;
}

void setPositions(struct entities all[], char extra[])
{
    char *ptr;
    int curId;
    bool running = true;

    //the actuall id
    ptr = strtok(extra, "&,");

    //the first element
    ptr = strtok(NULL, "&,");
    while (ptr != NULL)
    {
        curId = std::stoi(ptr);
        printf("id %s\n", ptr);

        ptr = strtok(NULL, "&,");
        printf("x %s\n", ptr);
        all[curId].cameraPos.x = std::stof(ptr);

        ptr = strtok(NULL, "&,");
        printf("y %s\n", ptr);
        all[curId].cameraPos.y = std::stof(ptr);

        ptr = strtok(NULL, "&,");
        printf("z %s\n", ptr);
        all[curId].cameraPos.z = std::stof(ptr);

        ptr = strtok(NULL, "&,");
    }


}
