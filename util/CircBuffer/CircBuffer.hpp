#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H
#include <cstring>
#include <stdio.h>
#include <cstdint>

class CircBuffer {

  public:
    CircBuffer(int packet_size, int packet_count, int fetch_size);
    void add(char *data);
    void add(char* data, int data_size);
    char* getData();
    void destroy();
    //TODO
    // returns the number of complete buffers we can get
    int fullBuffers();
    // if this.fullBuffers() <= NUM_BUFFERS - 1;
    int needMore();


  private:
    int _packet_size;
    int _packet_count;
    int _buffer_size;
    int _add_position;
    int _data_count;
    uintptr_t _remove_index;
    uintptr_t _fetch_size;
    char* _buffer;
    char* _output;
};

#endif
