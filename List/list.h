#ifndef LINKER_LIST_H
#define LINKER_LIST_H

#include <stdint.h>

#ifndef LIST_STRUCT
#define LIST_STRUCT
typedef struct listStruct
{
    void* arrayPtr;
    uint32_t currentLength;
    uint32_t allocatedMem;
    uint8_t elementSize;
} List;
#endif //LIST_STRUCT

extern List* newList(uint8_t elementSize);

extern void initList(void* memPtr, uint8_t elementSize);

extern void freeList(List*);

extern void add(List*, size_t element);

extern void removeElement(List*, size_t element);

extern void removeIndex(List*, size_t element);

extern uint32_t getIndex(List*, size_t element);

#endif //LINKER_LIST_H
