#include <stdio.h>
#include <stdint.h>

typedef struct
{
    float* _data;
    uint32_t _blockSize;
    uint32_t _writeIndex;
    uint32_t _readIndex;
} readBuffer;

readBuffer* createReadBuffer(uint32_t size);
void deleteReadBuffer(readBuffer* buff);

bool incBuffRead(readBuffer* buff);
void incBuffWrite(readBuffer* buff);

bool isOverflow(readBuffer* buff);
