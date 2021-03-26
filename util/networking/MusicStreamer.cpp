#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <thread>

#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <iostream>

using namespace std;

#include "networkConfig.hpp"
#include "stream.hpp"
#include "MusicStreamer.hpp"
#include "TCP.hpp"

Music::Music(char* ip) : TCP(ip, PORTTCP_MUSIC)
{
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &_w);
    _barWidth = _w.ws_col - 16;
}

void Music::run()
{
    if (!validate())
        printf("oh no!");

    struct musicHeader header;

    sendPTL(STARTSTREAM);

    bool requested = false;
    bool headReady = false;
    bool actuallyDone = false;
    bool ready = false;

    struct generalTCP& bufT = getInBuf();

    header.dataSize = 1;
    long cursor = 0;
    int numBuffers = 0;
    ALint state;
    int pollStatus = 0;
    bool stopUsingPoll = false;

    twitchStreamer player;

    //thread musicPlayer(threadRunner, bufT.data, ref(ready), ref(actuallyDone), ref(numBuffers), ref(state), ref(header), ref(headReady));
    thread progressBarThing(progressBarWithBufThread, ref(cursor), ref(header.dataSize), _barWidth, ref(numBuffers));
    progressBarThing.detach();

    printf("Starting\n");
    bool done = false;
    while (!done)
    {
        if (!ready && !stopUsingPoll)
            pollStatus = getFromPoll(true);

        if (!ready && pollStatus == 0)
        {
            if (bufT.protocol == SONGHEADER)
            {
                memcpy(&header, &bufT.data, sizeof(struct musicHeader));
                printf("\nchannels %d, sampleRate %d, bps %d, size %d\n",
                    header.channels, header.sampleRate,
                    header.bitsPerSample, header.dataSize);

                player.setHead(header);
            }
            else if (bufT.protocol == MORESONG)
            {
                // check that we are sending a file and that they want the next line
                if (bufT.numObjects != SOCKET_BUFF)
                    printf("\noh no\n");

                cursor += bufT.numObjects;
                ready = true;

                requested = false;
            }
            else if (bufT.protocol == ENDSONG)
            {
                printf("actually done with song\n");
                actuallyDone = true;
                cursor += bufT.numObjects;
                ready = true;

                sendPTL(ENDSONG);
            }
        }
        else if (pollStatus == POLLHUNGUP && !stopUsingPoll)
        {
            stopUsingPoll = true;
        }

        // make sure we are as full as can be
        if (numBuffers < (MUSIC_BUFFERS - 1) && !requested)
        {
            sendPTL(MORESONG);
            requested = true;
        }

        // music stuff
        numBuffers = player.getNumBuffers();
        state = player.getState();

        // add, normal, or add the remaining part
        if (numBuffers < MUSIC_BUFFERS - 1 && ready)
        {
            player.addBuffer(bufT.data);
            ready = false;
        }
        else if (done && numBuffers < MUSIC_BUFFERS && ready)
        {
            player.addBuffer(bufT.data);
            ready = false;
        }


        // if we have enough, then we play, or if we are done then force to play
        if ((numBuffers > START_MUSIC_BUFFERS && (state == AL_PAUSED || state == AL_INITIAL)) ||
            actuallyDone && (state == AL_PAUSED))
        {
            player.play();
            printf("\n============START===========\n");
        }

        if (numBuffers < MIN_MUSIC_BUFFERS && state == AL_PLAYING &&
            !actuallyDone)
        {
            player.pause();
            printf("\n============PAUSE===========\n");
        }

        // we are done, and the player is done
        if (actuallyDone && state == AL_STOPPED)
        {
            if (actuallyDone)
            {
                printf("done\n");
            }
            else
            {
                printf("we broke it\n");
            }

            actuallyDone = false;
            done = true;
        }
        else
        {
            this_thread::sleep_for(10ms);
        }
    }

    printf("Exit\n");
}

