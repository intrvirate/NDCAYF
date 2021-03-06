#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <thread>

#include <cstring>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <vector>
#include <bit>


#include "../networkConfig.hpp"
#include "stream.hpp"


//void update_stream(const ALuint source, const std::vector<char>& soundData, std::size_t& cursor, Buff& net);


#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)
#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
template<typename alcFunction, typename... Params>
auto alcCallImpl(const char* filename, 
                 const std::uint_fast32_t line, 
                 alcFunction function, 
                 ALCdevice* device, 
                 Params... params)
->typename std::enable_if_t<std::is_same_v<void,decltype(function(params...))>,bool>
{
    function(std::forward<Params>(params)...);
    return check_alc_errors(filename,line,device);
}

template<typename alcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char* filename,
                 const std::uint_fast32_t line,
                 alcFunction function,
                 ReturnType& returnValue,
                 ALCdevice* device, 
                 Params... params)
->typename std::enable_if_t<!std::is_same_v<void,decltype(function(params...))>,bool>
{
    returnValue = function(std::forward<Params>(params)...);
    return check_alc_errors(filename,line,device);
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename,
    const std::uint_fast32_t line,
    alFunction function,
    Params... params)
    ->typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>
{
    auto ret = function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
    return ret;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename,
    const std::uint_fast32_t line,
    alFunction function,
    Params... params)
    ->typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
{
    function(std::forward<Params>(params)...);
    return check_al_errors(filename, line);
}


/**
 * removes uesd buffers
 */
void twitchStreamer::clean()
{
    ALint buffersProcessed = 0;
    alCall(alGetSourcei, _source, AL_BUFFERS_PROCESSED, &buffersProcessed);

    if(buffersProcessed <= 0)
        return;

    ALuint buffer;
    alCall(alSourceUnqueueBuffers, _source, 1, &buffer);
}

/**
 * removes a buffer
 *
 */
void twitchStreamer::addBuffer(char* theData)
{

    ALint buffersProcessed = 0;
    alCall(alGetSourcei, _source, AL_BUFFERS_PROCESSED, &buffersProcessed);

    ALuint buffer;
    // remove a buffer if we can, other wise just set to the next
    //printf("remove %d\n", buffersProcessed);
    if(buffersProcessed > 0)
    {
        //printf("remove\n");
        alCall(alSourceUnqueueBuffers, _source, 1, &buffer);
    }
    else
    {
        if (_bufsIn > MUSIC_BUFFERS)
        {
            std::cerr << "Oh no" << std::endl;
        }
        buffer = _buffers[_bufsIn];
        _bufsIn++;
    }


    alCall(alBufferData, buffer, _header.format, theData, BUFFER_SIZE, _header.sampleRate);
    alCall(alSourceQueueBuffers, _source, 1, &buffer);
}

void twitchStreamer::initAdd(char* theData, int index)
{
    alCall(alBufferData, _buffers[index], _header.format, theData, BUFFER_SIZE, _header.sampleRate);
    alCall(alSourceQueueBuffers, _source, 1, &_buffers[index]);
    _bufsIn++;
}

int twitchStreamer::getNumBuffers()
{
    ALint buffersProcessed = 0;
    alCall(alGetSourcei, _source, AL_BUFFERS_PROCESSED, &buffersProcessed);

    return _bufsIn - buffersProcessed;
}

void twitchStreamer::play()
{
    alCall(alSourcePlay, _source);

    _state = AL_PLAYING;
}

void twitchStreamer::pause()
{
    alCall(alSourcePause, _source);

    _state = AL_PAUSED;
}

ALint twitchStreamer::getState()
{
    alCall(alGetSourcei, _source, AL_SOURCE_STATE, &_state);
    return _state;
}

void twitchStreamer::setHead(struct musicHeader theHead)
{
    _header = theHead;
}


void playLocalFile(std::string& song)
{
    struct musicHeader head;

    char* data = load_wav(song, head.channels,
        head.sampleRate, head.bitsPerSample, head.dataSize, head.format);

    printf("channels %d, sampleRate %d, bps %d, size %d\n\n",
        head.channels, head.sampleRate, head.bitsPerSample, head.dataSize);

    char buf[BUFFER_SIZE];
    bool ready = false;
    bool actuallyDone = false;
    bool headReady = true;
    int numBuffers = 0;
    long cursor = 0;
    ALint state = AL_INITIAL;

    std::thread musicPlayer(threadRunner, buf, std::ref(ready), std::ref(actuallyDone), std::ref(numBuffers), std::ref(state), std::ref(head), std::ref(headReady));

    bool running = true;
    while (running)
    {
        if (!ready)
        {
            if ((head.dataSize - (cursor + BUFFER_SIZE)) < 0)
            {
                memcpy(&buf, &data[cursor], head.dataSize - cursor);
                ready = true;
                actuallyDone = true;
                running = false;
            }
            else
            {
                memcpy(&buf, &data[cursor], BUFFER_SIZE);
                cursor += BUFFER_SIZE;
                ready = true;
            }
        }
    }

    musicPlayer.join();
}


void threadRunner(char* data, bool& ready, bool& done, int& numBuffers, ALint& state, struct musicHeader& header, bool& headReady)
{
    bool running = true;
    twitchStreamer player;

    while (running)
    {
        numBuffers = player.getNumBuffers();
        state = player.getState();

        // add, normal, or add the remaining part
        if (numBuffers < MUSIC_BUFFERS - 1 && ready)
        {
            player.addBuffer(data);
            ready = false;
        }
        else if (done && numBuffers < MUSIC_BUFFERS && ready)
        {
            player.addBuffer(data);
            ready = false;
        }

        if (headReady)
        {
            //printf("\nadded head\n");
            player.setHead(header);
            headReady = false;
        }


        // if we have enough, then we play, or if we are done then force to play
        if ((numBuffers > START_MUSIC_BUFFERS && (state == AL_PAUSED || state == AL_INITIAL)) ||
            done && (state == AL_PAUSED))
        {
            player.play();
            printf("\n============START===========\n");
        }

        if (numBuffers < MIN_MUSIC_BUFFERS && state == AL_PLAYING && !done)
        {
            player.pause();
            printf("\n============PAUSE===========\n");
        }

        // we are done, and the playing has stopped
        // then leave
        if (done && state == AL_STOPPED)
        {
            running = false;
        }
    }

    printf("dead\n");
}


/**
 * makes a audio player
 * @param head the music header info
 * @param buff a buffer with atleast 8 buffers
 */
twitchStreamer::twitchStreamer()
{
    _cursor = BUFFER_SIZE * MUSIC_BUFFERS;
    _bufsIn = 0;
    DONE = false;


    _openALDevice = alcOpenDevice(nullptr);
    if(!_openALDevice)
    {
        std::cerr << "ERROR: Could not create audio device" << std::endl;
        exit(EXIT_FAILURE);
    }


    if(!alcCall(alcCreateContext, _openALContext, _openALDevice, _openALDevice, nullptr) || !_openALContext)
    {
        std::cerr << "ERROR: Could not create audio context" << std::endl;
        exit(EXIT_FAILURE);
    }

    _contextMadeCurrent = false;
    if(!alcCall(alcMakeContextCurrent, _contextMadeCurrent, _openALDevice, _openALContext) || _contextMadeCurrent != ALC_TRUE)
    {
        std::cerr << "ERROR: Could not make audio context current" << std::endl;
        exit(EXIT_FAILURE);
    }


    alCall(alGenBuffers, MUSIC_BUFFERS, &_buffers[0]);

    alCall(alGenSources, 1, &_source);
    alCall(alSourcef, _source, AL_PITCH, 1);
    alCall(alSourcef, _source, AL_GAIN, 1.0f);
    alCall(alSource3f, _source, AL_POSITION, 0, 0, 0);
    alCall(alSource3f, _source, AL_VELOCITY, 0, 0, 0);
    alCall(alSourcei, _source, AL_LOOPING, AL_FALSE);

    //alCall(alSourceQueueBuffers, _source, MUSIC_BUFFERS, &_buffers[0]);

    _streamStatus = 0;

}


twitchStreamer::~twitchStreamer()
{
    alCall(alDeleteSources, 1, &_source);
    alCall(alDeleteBuffers, MUSIC_BUFFERS, &_buffers[0]);

    alcCall(alcMakeContextCurrent, _contextMadeCurrent, _openALDevice, nullptr);
    alcCall(alcDestroyContext, _openALDevice, _openALContext);

    ALCboolean closed;
    alcCall(alcCloseDevice, closed, _openALDevice, _openALDevice);
}


bool check_alc_errors(const std::string& filename, const std::uint_fast32_t line, ALCdevice* device)
{
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n" ;
        switch(error)
        {
        case ALC_INVALID_VALUE:
            std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case ALC_INVALID_DEVICE:
            std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
            break;
        case ALC_INVALID_CONTEXT:
            std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
            break;
        case ALC_INVALID_ENUM:
            std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
            break;
        case ALC_OUT_OF_MEMORY:
            std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
            break;
        default:
            std::cerr << "UNKNOWN ALC ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}



bool check_al_errors(const std::string& filename, const std::uint_fast32_t line)
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n" ;
        switch(error)
        {
        case AL_INVALID_NAME:
            std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            std::cerr << "UNKNOWN AL ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}



std::int32_t convert_to_int(char* buffer, std::size_t len)
{
    std::int32_t a = 0;
    if(std::endian::native == std::endian::little)
        std::memcpy(&a, buffer, len);
    else
        for(std::size_t i = 0; i < len; ++i)
            reinterpret_cast<char*>(&a)[3 - i] = buffer[i];
    return a;
}

//http://soundfile.sapp.org/doc/WaveFormat/
//read this and this function makes sense
bool load_wav_file_header(std::ifstream& file, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, ALsizei& size)
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
    size = convert_to_int(buffer, 4);

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

char* load_wav(const std::string& filename, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, ALsizei& size, ALenum& format)
{
    std::ifstream in(filename, std::ios::binary);
    if(!in.is_open())
    {
        std::cerr << "ERROR: Could not open \"" << filename << "\"" << std::endl;
        return nullptr;
    }
    if(!load_wav_file_header(in, channels, sampleRate, bitsPerSample, size))
    {
        std::cerr << "ERROR: Could not load wav header of \"" << filename << "\"" << std::endl;
        return nullptr;
    }

    char* data = new char[size];


    in.read(data, size);

    if (channels == 1 && bitsPerSample == 8)
        format = AL_FORMAT_MONO8;
    else if (channels == 1 && bitsPerSample == 16)
        format = AL_FORMAT_MONO16;
    else if (channels == 2 && bitsPerSample == 8)
        format = AL_FORMAT_STEREO8;
    else if (channels == 2 && bitsPerSample == 16)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr
            << "ERROR: unrecognized wave format: "
            << channels << " channels, "
            << bitsPerSample << " bps" << std::endl;
        exit(EXIT_FAILURE);
    }

    return data;
}
