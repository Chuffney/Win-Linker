#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "List/list.h"
#include "List/NSlist.h"
#include "pe.h"

static List nullTerminatedStrings;
static List linkedFiles;
static List finalSections;

#pragma pack(push, 1)
struct terminatedString //if a COFF symbol is exactly 8 bytes long it isn't null terminated and needs to be copied to a temporary buffer
{
    union
    {
        uint8_t string[8];
        uint64_t singleChunk;
    };
    uint8_t nullTerminator;
};

struct staticImportEntry
{
    uint32_t textOffset;
    uint32_t symbolIndex;
    uint16_t unknown;   //always value 4 todo: figure out if this is important
};
#pragma pack(pop)

struct objectFile
{
    char* fileName;
    char* contents;
    List* exportedSymbols;
    List* exportedSymbolsOffsets;
    List* importedSymbols;
    List* sections;
};

struct section
{
    char* name;
    uint32_t flags;
    uint32_t sizeOfRawData;
};

char* loadFile(const char* fileName)
{
    FILE* file = fopen(fileName, "rb");
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length);
    fread(buffer, 1, length, file);
    fclose(file);
    return buffer;
}

void processFiles(int argc, char** argv)
{
    uint32_t numberOfFiles = argc - 1;
    char** fileNames = &argv[1];
    for (uint32_t i = 0; i < numberOfFiles; i++)
    {
        struct objectFile newFile = {
                .fileName = fileNames[i],
                .contents = loadFile(fileNames[i]),
                .exportedSymbols = newList(sizeof(char*)),
                .exportedSymbolsOffsets = newList(sizeof(uint32_t)),
                .importedSymbols = newList(sizeof(char*)),
                .sections = newList(sizeof(struct section*))
        };
        NS_addRef(&linkedFiles, &newFile);
    }
}

static void initStaticMem()
{
    NS_initList(&nullTerminatedStrings, sizeof(struct terminatedString));
    NS_initList(&linkedFiles, sizeof(struct objectFile));
    NS_initList(&finalSections, sizeof(struct section));
}

static char* addTerminatedString(uint64_t string)
{
    struct terminatedString terminatedString = {.singleChunk = string, .nullTerminator = 0};
    return NS_addRef(&nullTerminatedStrings, &terminatedString);
}

static uint32_t asciiToBin(const char* asciiNumber, int length)
{
    uint32_t sum = 0;
    uint32_t powerOfTen = 1;
    for (int i = length - 1; i >= 0; i--)
    {
        sum += (asciiNumber[i] - '0') * powerOfTen;
        powerOfTen *= 10;
    }
    return sum;
}

void collectSymbols(struct objectFile* file)
{
    struct COFF_header* COFF_header = (struct COFF_header*) file->contents;
    uint32_t symbolCount = COFF_header->NumberOfSymbols;
    struct SymbolTableEntry* symbolTable = (struct SymbolTableEntry*) ((size_t) COFF_header + COFF_header->PointerToSymbolTable);
    uint8_t* stringTable = (uint8_t*) symbolTable + (symbolCount * sizeof(struct SymbolTableEntry));

    List* exportList = file->exportedSymbols;
    List* importList = file->importedSymbols;

    for (uint32_t i = 0; i < symbolCount; i++)
    {
        struct SymbolTableEntry* symbol = &symbolTable[i];
        List* listPtr;

        if (symbol->Type != SYMBOL_FUNCTION)
            continue;

        if (symbol->StorageClass == EXTERNAL_SYMBOL)
            listPtr = importList;
        else
            listPtr = exportList;

        if (symbol->Zeroes == 0)
        {
            uint32_t stringTableOffset = symbol->Offset;
            add(listPtr, (size_t) stringTable + stringTableOffset);
        } else
        {
            if (symbol->ShortName[7] == 0)
                add(listPtr, (size_t) symbol->ShortName);
            else
            {
                char* stringPtr = addTerminatedString(symbol->singleChunk);
                add(listPtr, (size_t) stringPtr);
            }
        }
    }

    struct Section_header* sectionTable = (struct Section_header*) (file->contents + COFF_header->SizeOfOptionalHeader + sizeof(struct COFF_header));
    uint16_t sectionCount = COFF_header->NumberOfSections;

    for (uint16_t i = 0; i < sectionCount; i++)
    {
        struct Section_header* sectionTableEntry = &sectionTable[i];
        struct section newSection;

        if (sectionTableEntry->Name.ShortName[0] != '/')    //short name
        {
            if (sectionTableEntry->Name.ShortName[7] == 0)  //needs no terminating
                newSection.name = (char*) &sectionTableEntry->Name;
            else
                newSection.name = addTerminatedString(sectionTableEntry->Name.singleChunk);
        } else  //long name in string table
        {
            uint32_t stringTableOffset = asciiToBin(&sectionTableEntry->Name.ShortName[1], 7);
            newSection.name = stringTable + stringTableOffset;
        }
        newSection.flags = sectionTableEntry->Characteristics;
        newSection.sizeOfRawData = sectionTableEntry->SizeOfRawData;
        add(file->sections, (size_t) &newSection);
    }
}

void collectSections(struct objectFile* file)
{
    struct COFF_header* COFF_header = (struct COFF_header*) file->contents;
    struct Section_header* sectionTable = (struct Section_header*) (file->contents + COFF_header->SizeOfOptionalHeader + sizeof(struct COFF_header));
    uint16_t sectionCount = COFF_header->NumberOfSections;

    uint32_t symbolCount = COFF_header->NumberOfSymbols;
    struct SymbolTableEntry* symbolTable = (struct SymbolTableEntry*) ((size_t) COFF_header + COFF_header->PointerToSymbolTable);
    uint8_t* stringTable = (uint8_t*) symbolTable + (symbolCount * sizeof(struct SymbolTableEntry));

    for (uint16_t i = 0; i < sectionCount; i++)
    {
        struct Section_header* sectionTableEntry = &sectionTable[i];
        struct section newSection = {
                .name = (char*) &sectionTableEntry->Name
        };
    }
}

int main(int argc, char** argv)
{
    initStaticMem();
    processFiles(argc, argv);
    for (int i = 0; i < linkedFiles.currentLength; i++)
    {
        struct objectFile file = ((struct objectFile*) linkedFiles.arrayPtr)[i];
        collectSymbols(&file);
        collectSections(&file);
    }

    free(nullTerminatedStrings.arrayPtr);
    free(linkedFiles.arrayPtr);
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
