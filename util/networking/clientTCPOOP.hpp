#ifndef CLIENTTCPOOP_H
#define CLIENTTCPOOP_H
#include <string>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

struct generalTCP
{
    char name[10];
    int protocol;
    int numObjects;
    size_t dataSize;
    struct timeval time;
    char data[SOCKET_BUFF];

};

/*
struct musicHeader
{
    uint8_t channels;
    int32_t sampleRate;
    uint8_t bitsPerSample;
    ALsizei dataSize;
};
*/

struct aboutFile
{
    char name[30];
    int type;
    long lines;
};

struct lines
{
    char aLine[200];
};

void progressBarThread(long& top, int& bottom, int width);
void progressBarWithBufThread(long& top, int& bottom, int width, int& numBuffs);
void drawProgress(double percent, int width);
void drawProgressWithBufCount(double percent, int width, int numBuffs);
void drawProgressRaw(double percent, int width);

class TCP {

  public:
    TCP(char* ip, int type, std::string filename);
    void run();
  private:
    int sockTCP;
    struct sockaddr_in tcpServer;
    socklen_t addrlen;
    size_t bufTSize;

    int makeTCP();
    bool tcpConnect(char ip[], int type);
    struct generalTCP makeBasicTCPPack(int ptl);
    int getLines(std::string file);


    void fileSendInit();
    int getFromPoll(bool waitForFill);
    bool waitForKey();
    void sendFileInfo(std::ifstream &myfile);
    bool sendNextLine(std::ifstream &myfile);
    bool fileSendMain();
    void musicInit();
    bool musicGet();
    void sendPTL(int protocol, int size);
    bool chance(int num);

    static void foobar(int& number);

    char* _ip;
    int _type;


    // packet we send
    struct generalTCP toSend;
    // packet we get
    struct generalTCP bufT;

    // info about the file
    struct aboutFile fileInfo;
    std::string fileName;
    long totalLine;

    struct lines bunch;

    // measure the length in time
    struct timeval before;
    struct timeval after;
    struct timeval diff;

    // for poll
    struct pollfd pfd;

    // for the progress bar
    struct winsize w;
    int barWidth;

    // figures
    int charsRead;
    long charsProcessed;

    // for tcp loop
    long count;
    bool done;
    int len;


    // file send specific
    bool sendingFile;
    bool waitingForTime;

    // for music streaming


};

#endif
