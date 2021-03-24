#ifndef TCP_H
#define TCP_H
#include <poll.h>

class TCP {
    public:
        bool waitForKey();
        void sendPTL(int protocol);
        int getFromPoll(bool waitForFill);
        struct generalTCP& getInBuf();
        struct generalTCP& getOutBuf();
        bool tcpConnect();
        TCP(char* ip, int port);

    private:
        struct generalTCP makeBasicTCPPack(int ptl);
        bool makeTCP();

        // the in out buffers
        struct generalTCP _toSend;
        struct generalTCP _toRecieve;

        // socket stuff
        struct pollfd _pfd;
        int _theSock;


        // address stuff
        char* _ip;
        int _port;
};

void progressBarThread(long& top, int& bottom, int width);
void progressBarWithBufThread(long& top, int& bottom, int width, int& numBuffs);
void drawProgress(double percent, int width);
void drawProgressWithBufCount(double percent, int width, int numBuffs);
void drawProgressRaw(double percent, int width);

#endif
