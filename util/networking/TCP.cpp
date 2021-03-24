#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>

using namespace std;
#include "networkConfig.hpp"
#include "TCP.hpp"


/**
 * makes a tcp socket
 * @return success or not
 */
bool TCP::makeTCP()
{
    bool success = true;
    if ((_theSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failed to make\n");
        success = false;
    }

    return success;
}


/**
 * tries to connect
 * @return whether we connected or not
 */
bool TCP::validate()
{
    bool successful = true;
    if (!tcpConnect())
    {
        successful = false;
        printf("oh no!\n");
    }

    if (!waitForKey())
    {
        successful = false;
        printf("didn't get key\n");
    }

    return successful;
}


/**
 * makes a tcp struct with the hostname and ptl
 * and returns, for ease of use
 * @param ptl what protocol this packet is
 * @return a struct with its header filled, mostly
 */
struct generalTCP TCP::makeBasicTCPPack(int ptl)
{
    struct generalTCP pack;
    gethostname(pack.name, 10);
    pack.protocol = ptl;
    pack.numObjects = 1;

    return pack;
}


/**
 * sends over the socket
 * @param protocol
 * @return
 */
void TCP::sendPTL(int protocol)
{
    _toSend.protocol = protocol;

    // send
    if (send(_theSock, (const void*)&_toSend, sizeof(struct generalTCP), 0) < 0)
    {
        perror("send wackiness");
    }
}


/**
 * waites for server to send the key
 * @return if we got one
 */
bool TCP::waitForKey()
{
    bool waiting = true;
    bool success = false;
    int len = 0;

    int keySize = sizeof(SUPERSECRETKEY_SERVER);
    char buff[keySize];

    int tries = 10000;
    printf("Waiting for key\n");
    while (waiting)
    {
        if (getFromPoll(false) == 0)
        {
            len = recv(_theSock, buff, keySize, 0);
            if (strcmp(buff, SUPERSECRETKEY_SERVER) == 0)
            {
                success = true;
                waiting = false;
                printf("Got the key\n");
            }
        }
        //tries--;
        if (tries == 0)
        {
            send(_theSock, SUPERSECRETKEY_CLIENT, sizeof(SUPERSECRETKEY_CLIENT), 0);
            tries = 10000;
        }
    }

    len = 0;
    return success;
}


/**
 * waits for poll to trigger, then error checks, and sets the buf packet
 * 0 for fine, 1 for hung up and -1 for bad
 * @param waitForFill to wait for the buffer to fill up or not
 * @return int of what happened
 */
int TCP::getFromPoll(bool waitForFill)
{
    int peek;
    int len;
    int packSize = sizeof(_toRecieve);
    if (poll(&_pfd, 1, 1000) > 0)
    {
        if (_theSock < 0)
        {
            printf("Socket: %d\n", _theSock);
            exit(-1);
        }
        peek = recv(_theSock, &_toRecieve, packSize, MSG_PEEK | MSG_DONTWAIT);

        // they broke the connection
        if (peek == 0)
        {
            printf("They hung up\n");

            return POLLHUNGUP;
        }

        // error
        if (peek < 0)
        {
            printf("error: %d\n", peek);
            printf("socket: %d\n", _theSock);
            perror("1msg error");
            exit(-1);
            return POLLBAD;
        }

        if (waitForFill)
        {
            if (peek < packSize)
            {
                _toRecieve.protocol = -1;
            }
            else
            {
                len = recv(_theSock, &_toRecieve, packSize, 0);

                /*
                printf("got it all; cur: %ld\n", peek);
                cout << "Got name: " << _toRecieve.name << endl;
                cout << "Got ptl: " << _toRecieve.protocol << endl;
                cout << "Got numO: " << _toRecieve.numObjects << endl;
                cout << "Got dataSize: " << _toRecieve.dataSize << endl;
                //cout << "\nGot time: " << _toRecieve.time << endl;
                //cout << "Got data: " << _toRecieve.data << endl;

                char* ptr = (char*)&_toRecieve;
                size_t kk = sizeof(struct generalTCP);
                int k = 0;

                while (kk--)
                {
                    if (k == 16)
                    {
                        k = 0;
                        printf("\n");
                    }
                    printf("%hhx ", *ptr++);
                }
                */

                if (len != packSize)
                {
                    printf("len %d vs _packSize %d\n", len, packSize);
                    perror("Oh no:\n");
                }
            }
        }
    }

    return POLLOK;
}


struct generalTCP& TCP::getInBuf()
{
    return _toRecieve;
}


struct generalTCP& TCP::getOutBuf()
{
    return _toSend;
}


/**
 * connects our tcp socket to the server, sends our key
 * @param ip the server ip
 * @return if we did it
 */
bool TCP::tcpConnect()
{
    bool success = true;
    struct sockaddr_in tcpServer;
    socklen_t addrlen = sizeof(tcpServer);

    memset(&tcpServer, 0, addrlen);
    tcpServer.sin_family = AF_INET;
    tcpServer.sin_addr.s_addr= inet_addr(_ip);
    tcpServer.sin_port =  htons(_port);

    // try to connect, if yes then send the key
    if (connect(_theSock, (struct sockaddr*)&tcpServer, addrlen) < 0)
    {
        perror("Connect problems\n");
        success = false;
    }
    else
    {
        printf("Sending key\n");
        send(_theSock, SUPERSECRETKEY_CLIENT, sizeof(SUPERSECRETKEY_CLIENT), 0);
    }

    return success;
}



/**
 * the constructor for the tcp thing
 * @param ip server ip
 * @param type what we are doing with the tcp socket
 * @param file filename for uploading
 */
TCP::TCP(char* ip, int port) : _ip(ip), _port(port)
{
    // make the socket
    makeTCP();

    // make our in out packets
    _toSend = makeBasicTCPPack(PING);
    _toRecieve = makeBasicTCPPack(PONG);

    // set poll var
    _pfd.fd = _theSock;
    _pfd.events = POLLIN | POLLHUP;
    _pfd.revents = 0;

}


/**
 * all things that inherit from tcp must make a run function
 * this function will be the main driver
 * should be able to be run on a thread
 */
void TCP::run()
{
    cout << "Oh no" << endl;
}









void progressBarThread(long& top, int& bottom, int width)
{
    while (top != bottom)
    {
        drawProgress((float) top / (float) bottom, width);
    }
}

void progressBarWithBufThread(long& top, int& bottom, int width, int& numBuffs)
{
    while (top != bottom)
    {
        drawProgressWithBufCount((float) top / (float) bottom, width, numBuffs);
        this_thread::sleep_for(500ms);
    }
}

void drawProgress(double percent, int width)
{
    drawProgressRaw(percent, width);
    cout << "\r";
    cout.flush();
}


void drawProgressWithBufCount(double percent, int width, int numBuffs)
{
    drawProgressRaw(percent, width);
    cout << " " << numBuffs << " / " << MUSIC_BUFFERS << "\r";
    cout.flush();
}


/**
 * makes the pretty progress bar
 * stolen from stack overflow btw
 * @param percent how far along we want this
 * @param width the max width
 */
void drawProgressRaw(double percent, int width)
{
    cout << "[";
    int pos = width * percent;
    for (int i = 0; i < width; ++i) {
        if (i < pos) cout << "=";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << int(percent * 100.0) << " %";
}
