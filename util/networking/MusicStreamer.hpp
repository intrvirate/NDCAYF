#ifndef STREAMER_H
#define STREAMER_H
#include <sys/ioctl.h>

#include "TCP.hpp"

class Music: public TCP {
    public:
        Music(char* ip);
        void run();
    private:
        bool validate();

        struct winsize _w;
        int _barWidth;

};


#endif
