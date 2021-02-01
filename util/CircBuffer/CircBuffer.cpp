#include "CircBuffer.hpp"
#include <cstring>
#include <stdio.h>
#include <cstdint>

/**
 * Creates a new Circular Buffer for packet size and count of packet size
 */
CircBuffer::CircBuffer(int packet_size, int packet_count, int fetch_size)
{
    _packet_size = packet_size;
    _packet_count = packet_count;
    _fetch_size = fetch_size;
    _buffer = new char[packet_size * packet_count];
    _output = new char[fetch_size];
    _add_position = 0;
    _remove_index = 0;
    _buffer_size = packet_size * packet_count;
}


/**
 * Adds data to the revolving buffer
 * @param data the pointer of the start of the packet to add
 * @param data_size the amount of data we are adding
 */
void CircBuffer::add(char* data, int data_size)
{
    int index = _add_position * data_size;
    memcpy(&_buffer[index], data, data_size);

    if (data_size != _packet_size)
        memset(&_buffer[index + data_size], 0, _packet_size - data_size);


    if (++_add_position >= _packet_count)
    {
        _add_position = 0;
    }
}

/**
 * Adds data to the revolving buffer, assumes adds normal amount
 * @param data the pointer of the start of the packet to add
 */
void CircBuffer::add(char* data)
{
    add(data, _packet_size);
}

/**
 * Moves data from the buffer to the output buffer in the amount specified in
 * the ctor
 */
char* CircBuffer::getData()
{
    intptr_t overflow = _remove_index + _fetch_size - _buffer_size;
    if (overflow < 0)
    {
        overflow = 0;
    }
    uintptr_t offset = _fetch_size - overflow;

    // Copy as much as you can from _buffer to _output
    memcpy(_output, &_buffer[_remove_index], offset);
    // If your fetch can't be fulfilled by what's left in the buffer, pull from
    // the beginning of the buffer.
    if (overflow > 0)
    {
        memcpy(&_output[overflow], &_buffer, overflow);
        _remove_index = overflow;
    }
    else
    {
        _remove_index = _remove_index + _fetch_size;
    }

    return _output;
}

/**
 * Deletes this Circular Buffer
 */
void CircBuffer::destroy()
{
    delete[] _buffer;
    delete[] _output;
}
