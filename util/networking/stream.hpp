#ifndef STREAM_H
#define STREAM_H
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include "networkConfig.hpp"
#include "songBuffer.hpp"

class twitchStreamer
{
    public:
      twitchStreamer(struct musicHeader head, BufferManager *buff);
      bool playLoop();
      void destroy();

    private:
      bool DONE;
      struct musicHeader _header;
      std::size_t _cursor;
      BufferManager* _buff;
      ALCdevice* _openALDevice;
      ALCcontext* _openALContext;
      ALCboolean _contextMadeCurrent;
      ALuint _buffers[MUSIC_BUFFERS];
      ALint _state;
      ALuint _source;
      void update_stream();

};

#endif
