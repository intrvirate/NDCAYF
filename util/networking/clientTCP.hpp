#ifndef CLIENTTCP_H
#define CLIENTTCP_H
#include <string>

int makeTCP();
bool tcpConnect(char ip[]);
struct generalTCP makeBasicTCPPack(int ptl);
int getLines(std::string file);
void drawProgress(double percent, int width);


struct generalTCP
{
    char name[10];
    unsigned short int protocol;
    unsigned short int numObjects;
    struct timeval time;
    char data[1000];
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

#endif
