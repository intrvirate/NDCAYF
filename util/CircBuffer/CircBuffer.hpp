#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H
#include <cstring>
#include <stdio.h>
#include <cstdint>

class CircBuffer {

  public:
    CircBuffer(int packet_size, int packet_count, int fetch_size);
    int add(char *data);
    //bool canAddMore();
    //bool needMore();
    //bool clearing();
    //bool isNextReady();
    //bool isFull();
    char* getData();
    //int getSize();
    //void noMore();
    //int qSize();
    void destroy();

  private:
    int _packet_size;
    int _packet_count;
    int _buffer_size;
    int _add_position;
    uintptr_t _remove_index;
    uintptr_t _fetch_size;
    char* _buffer;
    char* _output;
    //bool clear;
};

#endif
