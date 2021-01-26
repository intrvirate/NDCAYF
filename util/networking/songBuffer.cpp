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
    bufs.push(Buffer dog());
}


/**
 * adds data to the buffers
 * if data is too much it will recurse
 * @param data the data why are trying to add
 * @param amount the amount from that data we want to add(from end)
 * @return how much extra data we couldn't add
 */
int BufferManager::add(void *data, int amount)
{
    char* temp = new char[amount];

    // adds the right amount to temp, from the end of data
    memcpy(&temp, &data[sizeof(data) - amount], amount);

    // add to last buffer
    int overFlow = bufs.back().addData(temp);

    // check if we have more data at the end that didn't make it
    if (overFlow != 0 && canAddMore())
    {
        bufs.push(Buffer aNewBuf());
        overFlow = add(data, overFlow);
    }

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
 * checks if the buffer in front is full
 * @return if full or not
 */
bool BufferManager::isNextFull()
{
    return bufs.front().isFull();
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
 * checks if the given data will over flow this buffer
 * @param data data that we are checking
 * @return how much over if any
 */
int Buffer::checkSize(void* data)
{
    int overFlow = 0;
    int inSize = sizeof(data);

    if (inSize + numData > BUFFER_SIZE)
    {
        overFlow = BUFFER_SIZE - (inSize + numData);
    }

    return overFlow;
}


/**
 * adds as much as it can
 * @param data what we want to add
 * @return how much data we didn't add
 */
int Buffer::addData(void* data)
{
    int overFlow = checkSize(data);
    int dataSize = sizeof(data);

    if (overFlow == 0)
    {
        memcpy(&buf[numData], &data[0], dataSize);
        numData += dataSize;
    }
    else
    {
        memcpy(&buf[numData], &data[0], dataSize - overFlow);
        numData += dataSize - overFlow;
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
