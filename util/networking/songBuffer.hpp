#ifndef SONGBUFFER_H
#define SONGBUFFER_H
#include <queue>
#include "networkConfig.hpp"


class Buffer {
    public:
        Buffer();
        int add(char* data, int amount);
        char* getData();
        int getSize();
        bool isFull();
        void destroy();
    private:
        char* buf;
        int numData;
        int checkSize(int amount);
};

// interfaces with others, manages buffers
class BufferManager {

  public:
    BufferManager();
    int add(char *data, int amount);
    bool canAddMore();
    bool needMore();
    bool clearing();
    bool isNextReady();
    bool isFull();
    char* getData();
    int getSize();
    void noMore();
    int qSize();


  private:
    bool peak;
    bool low;

    // manages the data

    std::queue<Buffer> bufs;
    bool clear;
};


#endif
