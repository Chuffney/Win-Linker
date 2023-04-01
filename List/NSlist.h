/**
 * This implementation uses non-standard element sizes (instead of 1, 2, 4 or 8 bytes that are well supported by x86_64)
 */

#ifndef LINKER_NSLIST_H
#define LINKER_NSLIST_H

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

extern List* NS_newList(uint8_t elementSize);

extern void NS_initList(void* memPtr, uint8_t elementSize);

extern void freeList(List*);    //freeList is identical in standard and non-standard versions

extern void* NS_add(List*, size_t element);

extern void* NS_addRef(List*, void* elementPtr);

extern void NS_removeElement(List*, void* elementPtr);

extern void NS_removeIndex(List*, uint32_t index);

extern uint32_t NS_getIndex(List*, void* elementPtr);

#endif //LINKER_NSLIST_H
