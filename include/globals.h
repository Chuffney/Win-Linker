#ifndef LINKER_GLOBALS_H

#include <stdint.h>
#include <resizeableBuffer.h>

typedef struct
{
    char* name;
    uint32_t address;
    uint16_t section;
} ResolvedSymbol;

typedef struct
{
    uint64_t name;
    binaryBuffer buffer;
    uint32_t characteristics;
} ResolvedSection;

typedef struct
{
    char* name;
    uint32_t address;
    uint16_t section;
    uint16_t type;
} Relocation;

extern stringBuffer mainStringBuffer;
extern binaryBuffer symbolBuffer;
extern binaryBuffer sectionBuffer;
extern binaryBuffer relocationBuffer;

void error(const char* message, const char* extra);

#define LINKER_GLOBALS_H

#endif //LINKER_GLOBALS_H
