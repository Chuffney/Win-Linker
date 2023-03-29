#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "List/list.h"
#include "help.h"
#include "pe.h"

static List exportedSymbols;
static List nullTerminatedStrings;

struct terminatedSymbol //if a COFF symbol is exactly 8 bytes long it isn't null terminated and needs to be copied to a temporary buffer
{
    uint8_t string[8];
    uint8_t nullTerminator;
};

static inline void printHelp()
{
    printf("%s", helpMessage);
}

static inline void initStaticMem()
{
    initList(&exportedSymbols, sizeof(char*));
}

static inline void collectExported(const char* objectFile)
{
    struct COFF_header* header = (struct COFF_header*) objectFile;
    uint32_t symbolCount = header->NumberOfSymbols;
    struct SymbolTableEntry* symbolTable = (struct SymbolTableEntry*) (objectFile + header->PointerToSymbolTable);

    uint8_t* stringTable = (uint8_t*) symbolTable + (symbolCount * sizeof(struct SymbolTableEntry));

    for (uint32_t i = 0; i < symbolCount; i++)
    {
        struct SymbolTableEntry* symbol = &symbolTable[i];

        if (symbol->Type != SYMBOL_FUNCTION)
            continue;

        if (symbol->Zeroes == 0)
        {
            uint32_t tableOffset = symbol->Offset;
            add(&exportedSymbols, (size_t) stringTable + tableOffset);
        } else
        {
            add(&exportedSymbols, (size_t) symbol->ShortName);
        }
    }
}

static char* loadFile(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length);
    fread(buffer, 1, length, file);
    fclose(file);
    return buffer;
}

int main(int argc, char** argv)
{
    initStaticMem();
    char* memPtr = loadFile("main.o");
    collectExported(memPtr);
    for (int i = 0; i < exportedSymbols.currentLength; i++)
    {
        printf("%s", ((char**) exportedSymbols.arrayPtr)[i]);
        putchar(0xA);
    }
    free(memPtr);
    free(exportedSymbols.arrayPtr);

    return 0;
}

/* plan:
 *
 * collect exported symbols
 * collect and compare imported symbols
 * count and collect number of all sections
 * merge all comparable sections into their final forms (.text, .bss, .data, .rdata)
 * as this is being done - resolve new addresses to symbols
 * wrap the data in headers
 */
