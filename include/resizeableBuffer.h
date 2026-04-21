#ifndef LINKER_RESIZEABLEBUFFER_H
#define LINKER_RESIZEABLEBUFFER_H

#include <stdint.h>

#pragma pack(push, 4)
struct resizeableBuffer {
        void* bufferPtr;
        uint32_t allocMemory;
        uint32_t currentOffset;
    };
#pragma pack(pop)

extern void initBufferObject(void*);

typedef struct resizeableBuffer stringBuffer;
extern uint32_t addString(stringBuffer*, const char*);

typedef struct resizeableBuffer binaryBuffer;
extern uint32_t addChunk(binaryBuffer*, const void* chunk, uint32_t length);

#endif //LINKER_RESIZEABLEBUFFER_H
