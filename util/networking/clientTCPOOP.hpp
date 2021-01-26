#ifndef CLIENTTCPOOP_H
#define CLIENTTCPOOP_H
#include <string>
#include <poll.h>
#include <sys/ioctl.h>

struct generalTCP
{
    char name[10];
    int protocol;
    int numObjects;
    struct timeval time;
    char data[60000];
};

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


class TCP {

  public:
    TCP(char* ip, int type, std::string filename);
  private:
    int sockTCP;
    struct sockaddr_in tcpServer;
    socklen_t addrlen;
    size_t bufTSize;

    int makeTCP();
    bool tcpConnect(char ip[], int type);
    struct generalTCP makeBasicTCPPack(int ptl);
    int getLines(std::string file);
    void drawProgress(double percent, int width);
    void fileSendInit();
    int getFromPoll(bool waitForFill);
    bool waitForKey();
    void sendFileInfo(std::ifstream &myfile);
    bool sendNextLine(std::ifstream &myfile);
    bool fileSendMain();


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


};

#endif
