#ifndef STREAM_H
#define STREAM_H
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <fstream>
#include "networkConfig.hpp"
#include "songBuffer.hpp"

class twitchStreamer
{
    public:
      twitchStreamer();
      int playLoop();
      void destroy();
      int getNumBuffers();
      void addBuffer(char* data);
      void update_stream();
      void play();
      void pause();
      ALint getState();
      void setHead(struct musicHeader head);
      void clean();
      void initAdd(char* theData, int index);
      void threadRunner(char* data, int &dataLen, bool &done);

    private:
      int _curBuffer;
      int _streamStatus;
      int _bufsIn;
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
      std::ofstream* _file;

};

char* load_wav(const std::string& filename, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, ALsizei& size);
std::int32_t convert_to_int(char* buffer, std::size_t len);
bool load_wav_file_header(std::ifstream& file, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, ALsizei& size);
bool check_alc_errors(const std::string& filename, const std::uint_fast32_t line, ALCdevice* device);
bool check_al_errors(const std::string& filename, const std::uint_fast32_t line);

#endif
