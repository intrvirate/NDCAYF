#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <string>
#include <poll.h>
#include <iostream>
#include <fstream>
#include <string>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <thread>

using namespace std;

#include "networkConfig.hpp"
#include "client.hpp"
#include "clientTCPOOP.hpp"
#include "songBuffer.hpp"
#include "stream.hpp"

/**
 * makes a tcp socket
 * @return success or not
 */
int TCP::makeTCP()
{
    int success = 0;
    if ((sockTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failed to make\n");
        success = -1;
    }

    return success;
}

/**
 * the constructor for the tcp thing
 * @param ip server ip
 * @param type what we are doing with the tcp socket
 * @param file filename for uploading
 * TODO separate the construction from the running of the program
 */
TCP::TCP(char* ip, int type, string file)
{
    addrlen = sizeof(tcpServer);
    fileName = file;
    _ip = ip;
    _type = type;


    // amke the socket
    makeTCP();

}

void TCP::run()
{
    // try and connnect to the server
    if (!tcpConnect(_ip, _type))
    {
        printf("tcpError!\n");
        exit(EXIT_FAILURE);
    }


    // universal stuff
    gettimeofday(&before, NULL);

    // for the poll
    pfd.fd = sockTCP;
    pfd.events = POLLIN | POLLHUP;
    pfd.revents = 0;

    charsRead = 0;
    charsProcessed = 0;

    count = 0;
    done = false;

    bufTSize = sizeof(struct generalTCP);
    /*
    ifstream in("main.cpp", std::ios::binary);


    printf("start buf tests\n");

    BufferManager man;
    printf("init\n");
    char *data1 = new char[6000];

    for (int i = 0; i < 6000; i++)
    {
        data1[i] = 'a';
    }

    for (int i = 0; i < 6000; i++)
    {
        cout << data1[i];
    }
    cout << endl;


    ofstream out2("fakefile2", std::ios::binary);
    for (int i = 0; i < 11; i++)
    {
        if (!in.read(data1, 6000))
        {
            perror("uh oh\n");
        }
        out2.write(data1, 6000);
        man.add(data1, 6000);
    }
    out2.close();
    in.close();
    ofstream out("fakefile", std::ios::binary);
    printf("added\n");
    printf("Status: %s %s %s\n", man.canAddMore() ? "true" : "false", man.isNextReady() ? "true" : "false", man.isFull() ? "true" : "false");

    char* datas = man.getData();
    out.write(datas, 66000);
    out.close();


    printf("end buf tests\n");
    exit(EXIT_FAILURE);
    */

    // verify they are server
    if (!TCP::waitForKey())
    {
        perror("Failed to find key!\n");
        exit(EXIT_FAILURE);
    }

    // type specific set up and run
    if (_type == UPLOADFILE)
    {
        fileSendInit();
        printf("Uploading file!\n");
        fileSendMain();
    }
    else if (_type == DOWNLOADFILE)
    {
        printf("Downloading file!\n");
        exit(EXIT_FAILURE);
    }
    else if (_type == STREAMMUSIC)
    {
        musicInit();
        printf("Streaming music\n");
        musicGet();
    }
    else if (_type == STREAMVOICE)
    {
        printf("Voice channel\n");
        exit(EXIT_FAILURE);
    }
}


/**
 * inits file send specific vars and such
 * @param fileName the name of the file we are to open
 */
void TCP::fileSendInit()
{
    toSend = makeBasicTCPPack(SENDINGFILEHEADER);
    string dir = "obj/objects/";
    printf("%s..%s\n", dir.c_str(), fileName.c_str());
    fileName = dir + fileName;
    totalLine = getLines(fileName);

    ifstream in_file(fileName, ios::binary);
    in_file.seekg(0, ios::end);
    int fileSize = in_file.tellg();

    // file stuff
    fileInfo.lines = fileSize;
    strcpy(fileInfo.name, fileName.c_str());
    fileInfo.type = MAP;

    printf("name %s lines %ld\n", fileInfo.name, totalLine);


    // for the progress bar
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    barWidth = w.ws_col - 8;

    sendingFile = false;
    waitingForTime = false;
}


/**
 * stuff thats needed
 * @return
 */
void TCP::musicInit()
{

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
    if (poll(&pfd, 1, 1000) > 0)
    {
        if (sockTCP < 0)
        {
            printf("socket: %d\n", sockTCP);
            exit(-1);
        }
        peek = recv(sockTCP, &bufT, bufTSize, MSG_PEEK | MSG_DONTWAIT);
        //printf("%d peek\n", peek);

        // they broke the connection
        if (peek == 0)
        {
            printf("they hung up\n");
            if (_type != STREAMMUSIC)
                exit(EXIT_FAILURE);

            return POLLHUNGUP;
        }

        // error
        if (peek < 0)
        {
            printf("error: %d\n", peek);
            printf("socket: %d\n", sockTCP);
            perror("1msg error");
            exit(-1);
            return POLLBAD;
        }

        if (waitForFill)
        {
            if (peek < bufTSize)
            {
                //printf("waiting for it all\n");
                bufT.protocol = -1;
            }
            else
            {
                len = recv(sockTCP, &bufT, bufTSize, 0);
                if (len != bufTSize)
                {
                    perror("Oh no:\n");
                }
            }
        }
    }

    return POLLOK;
}

/**
 * waites for server to send the key
 * @return if we got one
 * TODO make this only try x amount of times before failing
 */
bool TCP::waitForKey()
{
    bool waiting = true;
    bool success = false;
    char buff[2048];
    int tries = 10000;
    printf("waiting for key\n");
    while (waiting)
    {
        if (getFromPoll(false) == 0)
        {
            len = recv(sockTCP, buff, 2048, 0);
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
            send(sockTCP, SUPERSECRETKEY_CLIENT, sizeof(SUPERSECRETKEY_CLIENT), 0);
            tries = 10000;
        }
    }

    len = 0;
    return success;
}

/**
 * opens the file and gets the proper info
 * then makes aboutfile struct and sends it to server
 * @param myfile the file stream we are working with
 */
void TCP::sendFileInfo(ifstream &myfile)
{
    memcpy(&toSend.data, &fileInfo, sizeof(aboutFile));
    send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);

    toSend.protocol = SENDINGFILE;

    printf("Sent the info about the map\n");
    myfile.open(fileName, std::ios::out);

    if (!myfile.is_open())
    {
        perror("Error opening the file:");
    }
}

/**
 * sends the next line
 * if the buffer fills up then nothing special
 * but if not then we send with ENDDOWNLOAD ptl
 * @return
 */
bool TCP::sendNextLine(ifstream &myfile)
{
    // fillup the buffer with info from the file
    if (myfile.read(toSend.data, sizeof(toSend.data)))
    {
        // it filled up correctly
        charsRead = myfile.gcount(); // should be 1000
        count += charsRead;
        toSend.numObjects = charsRead;
        drawProgress((double)count / (double)fileInfo.lines, barWidth);


        // send
        if (send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0) < 0)
        {
            perror("send wackiness");
        }
    }
    else
    {
        // the buffer did not fully fill, at the end

        // should be <1000
        charsRead = myfile.gcount();
        count += charsRead;
        toSend.numObjects = charsRead;

        drawProgress(1.0f, barWidth);

        //printf(" %lu%03lu.%03lu milliseconds \n", diff.tv_sec, diff.tv_usec / 1000 , diff.tv_usec % 1000);

        toSend.protocol = ENDDOWNLOAD;
        send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);

        gettimeofday(&after, NULL);
        timersub(&after, &before, &diff);

        printf("\nSent all %ld byes in", count);
        printf(" %lu.%06lu seconds \n", diff.tv_sec, diff.tv_usec);

        myfile.close();

        waitingForTime = true;
        sendingFile = false;
    }

    return true;
}


/**
 * main loop for sending files
 * assumes the key has been got
 * preps file and sends the first line before entering
 * @return honestly not necessary
 */
bool TCP::fileSendMain()
{
    ifstream myfile;
    bool done = false;

    // prep file and send
    sendFileInfo(myfile);

    bool first = true;
    sendingFile = true;

    printf("starting\n");
    while (!done)
    {
        if (getFromPoll(true) == 0)
        {
            if (first)
            {
                sendNextLine(myfile);
                first = false;
            }
            // check that we are sending a file and that they want the next line
            if (sendingFile && (bufT.protocol == NEXTLINE))
            {
                sendNextLine(myfile);
            }
            else if (waitingForTime && (bufT.protocol == ENDDOWNLOAD))
            {
                timersub(&bufT.time, &before, &diff);
                printf("Server took %lu.%06lu seconds to finish\n", diff.tv_sec, diff.tv_usec);

                done = true;
                printf("exit\n");
                close(sockTCP);
            }
        }
    }

    return true;
}

void TCP::foobar(int& number)
{
    printf("a number %d\n", number);
}

bool TCP::chance(int num)
{
    int value;
    if (num > (MUSIC_BUFFERS - 50))
    {
        value = rand() % 70;
    }
    else if (num > (MUSIC_BUFFERS - 100))
    {
        value = rand() % 60;
    }
    else if (num > (MUSIC_BUFFERS - 100))
    {
        value = rand() % 50;
    }
    else if (num > (MUSIC_BUFFERS - 150))
    {
        value = rand() % 40;
    }
    else if (num > (MUSIC_BUFFERS - 200))
    {
        value = rand() % 30;
    }
    else if (num < (MUSIC_BUFFERS - 200))
    {
        value = 0;
    }

    if (value == 0)
        return true;

    return false;
}

/**
 * main loop for sending files
 * assumes the key has been got
 * preps file and sends the first line before entering
 * @return honestly not necessary
 */
bool TCP::musicGet()
{
    //twitchStreamer player;
    //player.getNumBuffers();

    bool done = false;
    struct musicHeader header;
    bool headReady = false;

    size_t cursor = 0;
    char temp[44];

    sendPTL(STARTSTREAM, 0);
    bool firstSong = true;
    sendingFile = true;
    bool requested = false;
    bool havePlayer = false;

    bool actuallyDone = false;
    bool ready = false;

    int numBuffers = 0;
    ALint state;

    int id = 0;
    bool notStarted = true;

    //thread musicPlayer(threadRunner, bufT.data, ref(ready), ref(actuallyDone), ref(player));
    thread musicPlayer(threadRunner, bufT.data, ref(ready), ref(actuallyDone), ref(numBuffers), ref(state), ref(header), ref(headReady));
    musicPlayer.detach();

    printf("starting\n");
    while (!done)
    {
        if (!ready && getFromPoll(true) == 0)
        {
            if (bufT.protocol == SONGHEADER)
            {
                // make audio player
                if (firstSong)
                {

                    firstSong = false;
                    memcpy(&header, &bufT.data, sizeof(struct musicHeader));
                    //player.setHead(header);
                    headReady = true;
                    printf("channels %d, sampleRate %d, bps %d, size %d\n\n",
                        header.channels, header.sampleRate,
                        header.bitsPerSample, header.dataSize);
                }
                else
                {
                    // add buf to end
                    //dump final info into the stream
                }
                requested = false;
                // do stuff
            }
            else if (bufT.protocol == MORESONG)
            {
                // check that we are sending a file and that they want the next line
                cursor += bufT.dataSize;
                //printf("\t\tmoresong\r");
                //std::cout.flush();
                ready = true;

                requested = false;

                /*
                memcpy(&current[curSize], &bufT.data, bufT.dataSize);
                curSize += bufT.dataSize;

                if (curSize != BUFFER_SIZE)
                {
                    // we want to fill this up
                    sendPTL(MORESONG, cursor);
                    requested = true;
                }
                else
                {
                    // wait
                    requested = false;
                }
                */


            }
            else if (bufT.protocol == ENDSONG)
            {
                printf("\nin size %ld\n", bufT.dataSize);
                actuallyDone = true;
                cursor += bufT.dataSize;
                ready = true;

                /*
                memcpy(&current[curSize], &bufT.data, bufT.dataSize);
                curSize += bufT.dataSize;

                // clear out the rest of buffer
                memset(&current[curSize], 0, BUFFER_SIZE - curSize);
                */

                sendPTL(ENDSONG, 0);
            }
        }
        // do music code
        //numBuffers = player.getNumBuffers();
        //state = player.getState();

        //player.clean();

        // make sure we are as full as can be
        printf("CurrentBufs: %d\r", numBuffers);
        std::cout.flush();
        if (numBuffers < (MUSIC_BUFFERS - 1) && !requested)
        {
            sendPTL(MORESONG, cursor);
            //printf("request\r");
            //std::cout.flush();
            requested = true;
        }

        /*
        if (numBuffers < MUSIC_BUFFERS && curSize == BUFFER_SIZE)
        {
            player.addBuffer(current);
            curSize = 0;
            printf("addbuffer at: %d\r", numBuffers);
            std::cout.flush();
        }
        */

        /*
        if ((numBuffers > 100 && (state == AL_PAUSED || state == AL_INITIAL)) ||
            actuallyDone && (state == AL_PAUSED))
        {
            player.play();
            printf("============START===========\n");
        }

        if (numBuffers < 20 && state == AL_PLAYING && !actuallyDone)
        {
            player.pause();
            printf("============PAUSE===========\n");
        }
        */


        if (actuallyDone && state == AL_STOPPED)
        {
            if (actuallyDone)
            {
                printf("done\n");
            }
            else
            {
                printf("we broke it\n");
            }

            actuallyDone = false;
            done = true;

            // reset the player, vars, idk
        }
    }

    //delete player;

    printf("exit\n");
    close(sockTCP);
    return true;
}


void TCP::sendPTL(int protocol, int size)
{
    toSend.protocol = protocol;
    toSend.dataSize = size;

    // send
    if (send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0) < 0)
    {
        perror("send wackiness");
    }
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
    strcpy(pack.name, hostnameGet());
    pack.protocol = ptl;
    pack.numObjects = 1;

    return pack;
}


/**
 * gets the lines of a file by brute force
 * @param file file we are opening
 * @return the number of lines in file
 */
int TCP::getLines(string file)
{
    ifstream fileToCount;
    int count = 0;
    string line;

    fileToCount.open(file);

    while (getline(fileToCount, line))
    {
        count++;
    }

    fileToCount.close();

    return count;
}


/**
 * makes the pretty progress bar
 * stolen from stack overflow btw
 * @param percent how far along we want this
 * @param width the max width
 */
void TCP::drawProgress(double percent, int width)
{
    cout << "[";
    int pos = width * percent;
    for (int i = 0; i < width; ++i) {
        if (i < pos) cout << "=";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << int(percent * 100.0) << " %\r";
    cout.flush();
}


/**
 * connects our tcp socket to the server, sends our key
 * @param ip the server ip
 * @return if we did it
 */
bool TCP::tcpConnect(char ip[], int type)
{

    int port = -1;
    if (type == UPLOADFILE)
    {
        port = PORTTCP_UPLOAD;
    }
    else if (type == DOWNLOADFILE)
    {
        port = PORTTCP_DOWNLOAD;
    }
    else if (type == STREAMMUSIC)
    {
        port = PORTTCP_MUSIC;
    }
    else if (type == STREAMVOICE)
    {
        port = PORTTCP_VOICE;
    }


    hostnameSet();
    bool success = true;
    memset(&tcpServer, 0, sizeof(tcpServer));
    tcpServer.sin_family = AF_INET;
    tcpServer.sin_addr.s_addr= inet_addr(ip);
    tcpServer.sin_port =  htons(port);

    // try to connect, if yes then send the key
    if (connect(sockTCP, (struct sockaddr*)&tcpServer, addrlen) < 0)
    {
        perror("Connect problems\n");
        success = false;
    }
    else
    {
        printf("sending key\n");
        send(sockTCP, SUPERSECRETKEY_CLIENT, sizeof(SUPERSECRETKEY_CLIENT), 0);
    }

    return success;
}
