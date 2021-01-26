#ifndef SONGBUFFER_H
#define SONGBUFFER_H
#include <queue>
#include "networkConfig.hpp"

// interfaces with others, manages buffers
class BufferManager {

  public:
    BufferManager();
    int add(void *data, int amount);
    char* popBuffer();
    bool canAddMore();
    bool isNextFull();
    bool isFull();
  private:
    std::queue<Buffer> bufs;

    // manages the data
    class Buffer {
        public:
            Buffer();
            int add(void* data);
            char* getData();
            bool isFull();
        private:
            char buf[BUFFER_SIZE];
            int numData;
            int checkSize(void* data);
    };

};


#endif
