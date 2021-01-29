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

#include "AudioFile.h"
#include "bufferGet.hpp"
#include "projectConsts.hpp"

/*
class Buff {

  public:
    Buff(std::string filename);
    std:uint8_t gChannels();
    std::int32_t gSampleRate();
    std::uint8_t gBitsPerSample();
    ALsizei     gDataSize();
    AudioFile<double> gAudioFile();
  private:
    bool load_wav_file_header(std::ifstream& file, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, ALsizei& size);
    char* load_wav(const std::string& filename, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, ALsizei& size)
    std::uint8_t channels;
    std::int32_t sampleRate;
    std::uint8_t bitsPerSample;
    ALsizei     dataSize;
    AudioFile<double> audioFile;
    bool done = false;
    ALuint buffers[NUM_BUFFERS];
    char* data;

};
*/

std::uint8_t Buff::gChannels()
{
    return channels;
}

std::int32_t Buff::gSampleRate()
{
    return sampleRate;
}

std::uint8_t Buff::gBitsPerSample()
{
    return bitsPerSample;
}

ALsizei     Buff::gDataSize()
{
    return dataSize;
}

AudioFile<double> Buff::gAudioFile()
{
    return audioFile;
}

ALenum Buff::gFormat()
{
    return format;
}

/**
 * load the file into a var, init our buffer stuff
 * @param filename the name of the file we will take data from
 */
Buff::Buff(std::string filename)
{
    //char* rawSoundData = load_wav(filename.c_str(), channels, sampleRate, bitsPerSample, dataSize);
    load_wav(filename.c_str());
    if(rawSoundData == nullptr || dataSize == 0)
    {
        std::cerr << "ERROR: Could not load wav" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<char> soundData(rawSoundData, rawSoundData + dataSize);
    printf("%d\n", bitsPerSample);

    audioFile.load(filename.c_str());
    audioFile.printSummary();
    double time = audioFile.getLengthInSeconds();
    int seconds = (int)(time) % 60;
    int minutes = (int)(time) / 60;
    std::cout << minutes << ":" << seconds << std::endl;

    std::cout << "Size: " << dataSize << std::endl;
    printf("channels %d, sampleRate %d, bps %d, size %d\n", channels, sampleRate, bitsPerSample, dataSize);


    if(channels == 1 && bitsPerSample == 8)
        format = AL_FORMAT_MONO8;
    else if(channels == 1 && bitsPerSample == 16)
        format = AL_FORMAT_MONO16;
    else if(channels == 2 && bitsPerSample == 8)
        format = AL_FORMAT_STEREO8;
    else if(channels == 2 && bitsPerSample == 16)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr
            << "ERROR: unrecognised wave format: "
            << channels << " channels, "
            << bitsPerSample << " bps" << std::endl;
        exit(EXIT_FAILURE);
    }
}


std::int32_t Buff::convert_to_int(char* buffer, std::size_t len)
{
    std::int32_t a = 0;
    if(std::endian::native == std::endian::little)
        std::memcpy(&a, buffer, len);
    else
        for(std::size_t i = 0; i < len; ++i)
            reinterpret_cast<char*>(&a)[3 - i] = buffer[i];
    return a;
}


/**
 *
 * @return
 */
char* Buff::firstBuffs()
{
    buff = new char[NUM_BUFFERS * BUFFER_SIZE];
    std::memcpy(&buff[0], &rawSoundData[0], NUM_BUFFERS * BUFFER_SIZE);
    return buff;
}



/**
 * a buffer of the appropriate size, and from the right spot
 * @param cursor where we get the data from
 * @param amount how much data
 * @return the buffer
 */
char* Buff::getBuff(std::size_t cursor, std::size_t amount)
{
    buff = new char[amount];
    std::memcpy(&buff[0], &rawSoundData[cursor], amount);
    return buff;
}

//http://soundfile.sapp.org/doc/WaveFormat/
//read this and this function makes sense
// gets the proper data out of the wav file
bool Buff::load_wav_file_header(std::ifstream& file)
{
    char buffer[4];
    if(!file.is_open())
        return false;

    // the RIFF
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read RIFF" << std::endl;
        return false;
    }


    if(std::strncmp(buffer, "RIFF", 4) != 0)
    {
        std::cerr << "ERROR: file is not a valid WAVE file (header doesn't begin with RIFF)" << std::endl;
        return false;
    }

    // the size of the file
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read size of file" << std::endl;
        return false;
    }

    // the WAVE
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read WAVE" << std::endl;
        return false;
    }
    if(std::strncmp(buffer, "WAVE", 4) != 0)
    {
        std::cerr << "ERROR: file is not a valid WAVE file (header doesn't contain WAVE)" << std::endl;
        return false;
    }

    // "fmt/0"
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read fmt/0" << std::endl;
        return false;
    }

    // this is always 16, the size of the fmt data chunk
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read the 16" << std::endl;
        return false;
    }

    // PCM should be 1?
    if(!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read PCM" << std::endl;
        return false;
    }

    // the number of channels
    if(!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read number of channels" << std::endl;
        return false;
    }
    channels = convert_to_int(buffer, 2);

    // sample rate
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read sample rate" << std::endl;
        return false;
    }
    sampleRate = convert_to_int(buffer, 4);

    // (sampleRate * bitsPerSample * channels) / 8
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read (sampleRate * bitsPerSample * channels) / 8" << std::endl;
        return false;
    }

    // ?? dafaq
    if(!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read dafaq" << std::endl;
        return false;
    }

    // bitsPerSample
    if(!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read bits per sample" << std::endl;
        return false;
    }
    bitsPerSample = convert_to_int(buffer, 2);
    //std::cerr << bitsPerSample << std::endl;

    // data chunk header "data"
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read data chunk header" << std::endl;
        return false;
    }

    //std::cerr << buffer << std::endl;
    //file.read(buffer, 
    if(std::strncmp(buffer, "data", 4) != 0)
    {
        std::cerr << "ERROR: file is not a valid WAVE file (doesn't have 'data' tag)" << std::endl;
        return false;
    }

    // size of data
    if(!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read data size" << std::endl;
        return false;
    }
    dataSize = convert_to_int(buffer, 4);

    /* cannot be at the end of file */
    if(file.eof())
    {
        std::cerr << "ERROR: reached EOF on the file" << std::endl;
        return false;
    }
    if(file.fail())
    {
        std::cerr << "ERROR: fail state set on the file" << std::endl;
        return false;
    }

    return true;
}

/**
 * loads the wav file
 *
 */
char* Buff::load_wav(const std::string& filename)
{
    std::ifstream in(filename, std::ios::binary);
    if(!in.is_open())
    {
        std::cerr << "ERROR: Could not open \"" << filename << "\"" << std::endl;
        return nullptr;
    }
    if(!load_wav_file_header(in))
    {
        std::cerr << "ERROR: Could not load wav header of \"" << filename << "\"" << std::endl;
        return nullptr;
    }

    rawSoundData = new char[dataSize];

    in.read(rawSoundData, dataSize);

    return rawSoundData;
}
