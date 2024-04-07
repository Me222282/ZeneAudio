#include <stdio.h>
#include "readBuffer.h"

readBuffer* createReadBuffer(uint32_t size)
{
    readBuffer* buff = new readBuffer();
    buff->_data = new float[size];
    buff->_blockSize = size;
    return buff;
}
void deleteReadBuffer(readBuffer* buff)
{
    delete buff->_data;
    delete buff;
}

bool incBuffRead(readBuffer* buff)
{
    buff->_readIndex++;
    if (buff->_readIndex >= buff->_blockSize)
    {
        buff->_readIndex = 0;
    }
    
    return buff->_readIndex == buff->_writeIndex;
}
void incBuffWrite(readBuffer* buff)
{
    buff->_writeIndex++;
    if (buff->_writeIndex >= buff->_blockSize)
    {
        buff->_writeIndex = 0;
    }
}

bool isOverflow(readBuffer* buff)
{
    return buff->_readIndex == buff->_writeIndex;
}
