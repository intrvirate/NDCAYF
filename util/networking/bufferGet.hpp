#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

#include <cstring>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <vector>
#include <bit>

#include "projectConsts.hpp"

class Buff {

  public:
    Buff(std::string filename);
    std::uint8_t gChannels();
    std::int32_t gSampleRate();
    std::uint8_t gBitsPerSample();
    ALsizei     gDataSize();
    AudioFile<double> gAudioFile();
    ALenum gFormat();
    char* firstBuffs();
    char* getBuff(std::size_t cursor, std::size_t amount);
  private:
    std::int32_t convert_to_int(char* buffer, std::size_t len);
    bool load_wav_file_header(std::ifstream& file);
    char* load_wav(const std::string& filename);
    std::uint8_t channels;
    std::int32_t sampleRate;
    std::uint8_t bitsPerSample;
    ALsizei     dataSize;
    AudioFile<double> audioFile;
    bool done = false;
    ALuint buffers[NUM_BUFFERS];
    char* rawSoundData;
    char* buff;
    ALenum format;

};
