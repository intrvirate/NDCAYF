#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <string>
#include <poll.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "networkConfig.hpp"
#include "client.hpp"
#include "clientTCPOOP.hpp"
#include "songBuffer.hpp"


/**
 * makes a buffer manager
 */
BufferManager::BufferManager()
{
    bufs.emplace();
    clear = false;
}


/**
 * tells this buffer to clear out
 */
void BufferManager::noMore()
{
    clear = true;
}

bool BufferManager::clearing()
{
    return clear;
}


bool BufferManager::needMore()
{
    return bufs.size() <= NUM_BUFFERS - 2;
}


/**
 * adds data to the buffers
 * if data is too much it will recurse
 * @param data the data why are trying to add
 * @param amount the amount from that data we want to add(from end)
 * @return how much extra data we couldn't add
 */
int BufferManager::add(char* data, int amount)
{
    char* temp = new char[amount];

    // adds the right amount to temp, from the end of data
    memcpy(temp, data, amount);

    // add to last buffer
    int overFlow = bufs.back().add(temp, amount);

    // check if we have more data at the end that didn't make it
    if (overFlow != 0 && !isFull())
    {
        bufs.emplace();
        overFlow = add(data, overFlow);
    }
    delete[] temp;

    return overFlow;
}


/**
 * checks if we can add more data to the buffers
 * wont keep anymore than max buffers
 * @return if we have room
 */
bool BufferManager::canAddMore()
{
    return bufs.size() <= NUM_BUFFERS && !bufs.back().isFull();
}


/**
 * gets the next buffer and returns its contents
 * @return buffer contents
 */
char* BufferManager::getData()
{
    char* temp = bufs.front().getData();
    bufs.pop();
    return temp;
}

/**
 * the size of the data
 * @return size of data
 */
int BufferManager::getSize()
{
    return bufs.front().getSize();
}


/**
 *
 * @return
 */
int BufferManager::qSize()
{
    return bufs.size();
}

/**
 * checks if the buffer in front is full
 * @return if full or not
 */
bool BufferManager::isNextReady()
{
    return bufs.front().isFull() || clear;
}


/**
 * if we are full
 * @return a bool of if we are
 */
bool BufferManager::isFull()
{
    return bufs.size() == NUM_BUFFERS;
}





/**
 * make a buffer
 * init vars
 */
Buffer::Buffer()
{
    numData = 0;
    buf = new char[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);
}

/**
 * the amount of data in buffer
 * @return int of size of data
 */
int Buffer::getSize()
{
    return numData;
}


/**
 * checks if the given data will over flow this buffer
 * @param data data that we are checking
 * @return how much over if any
 */
int Buffer::checkSize(int amount)
{
    int overFlow = 0;

    if ((amount + numData) > BUFFER_SIZE)
    {
        overFlow = (amount + numData) - BUFFER_SIZE;
    }

    return overFlow;
}


/**
 * adds as much as it can
 * @param data what we want to add
 * @return how much data we didn't add
 */
int Buffer::add(char* data, int amount)
{
    int overFlow = checkSize(amount);

    if (overFlow == 0)
    {
        memcpy(&buf[numData], data, amount);
        numData += amount;
    }
    else
    {
        memcpy(&buf[numData], data, amount - overFlow);
        numData += amount - overFlow;
    }

    return overFlow;
}


/**
 * returns the contents of the buffer
 * @return a char array of this buffer
 */
char* Buffer::getData()
{
    return buf;
}


/**
 * if this buffer is full
 * @return whether or not this buffer is full
 */
bool Buffer::isFull()
{
    return numData == BUFFER_SIZE;
}

/**
 * destroy allocations
 */
void Buffer::destroy()
{
    delete[] buf;
}
