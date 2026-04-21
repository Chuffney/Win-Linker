#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <resizeableBuffer.h>
#include <pe.h>
#include <stdbool.h>
#include <globals.h>
#include <emission.h>

stringBuffer mainStringBuffer;
binaryBuffer symbolBuffer;
binaryBuffer sectionBuffer;
binaryBuffer relocationBuffer;

void error(const char* message, const char* extra)
{
    fprintf(stderr, "%s ", message);
    fprintf(stderr, "%s\n", extra);

    exit(1);
}

static void initBuffers(void)
{
    initBufferObject(&mainStringBuffer);
    initBufferObject(&symbolBuffer);
    initBufferObject(&sectionBuffer);
    initBufferObject(&relocationBuffer);
}

static char* loadFile(const char* fileName)
{
    FILE* file = fopen(fileName, "rb");

    if (file == NULL)
        error("file not found", fileName);

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length);
    fread(buffer, 1, length, file);
    fclose(file);
    return buffer;
}


static char* resolveSymbolName(const struct COFF_symbol* symbol, const char* COFF_stringTable)
{
    uint32_t stringOffset;
    if (symbol->Name.Zeroes == 0)   //long name
    {
        stringOffset = addString(&mainStringBuffer, COFF_stringTable + symbol->Name.Offset);
    } else  //short name
    {
        struct nullTerminatedString
        {
            uint64_t chars;
            uint8_t terminator;
        };

        struct nullTerminatedString temporaryString = {
                .chars = symbol->Name.ShortName,
                .terminator = 0
        };
        stringOffset = addString(&mainStringBuffer, (char*) &temporaryString);
    }
    return mainStringBuffer.bufferPtr + stringOffset;
}

static uint64_t truncatedSectionName(const struct Section_header* section, const char* COFF_stringTable)
{
    struct nullTerminatedString
    {
        uint64_t chars;
        uint8_t terminator;
    };

    struct nullTerminatedString temporaryString = {
            .chars = section->Name,
            .terminator = 0
    };

    if ((char) section->Name == '/') //long name - ascii decimal offset into string table
    {
        uint32_t stringTableOffset = atoi((char*) &temporaryString + 1);
        temporaryString.chars = 0;
        strcpy_s((char*) &temporaryString, 8, COFF_stringTable + stringTableOffset);
        return temporaryString.chars;
    } else
        return section->Name; //short name
}

static ResolvedSection* checkExistingSections(struct Section_header* section, const char* COFF_stringTable)
{
    ResolvedSection* sectionArray = sectionBuffer.bufferPtr;
    uint64_t truncatedName = truncatedSectionName(section, COFF_stringTable);
    for (uint32_t bufferIterator = 0; bufferIterator < sectionBuffer.currentOffset / sizeof(ResolvedSection); bufferIterator++)
    {
        if (truncatedName == sectionArray[bufferIterator].name)
        {
            if (section->Characteristics != sectionArray[bufferIterator].characteristics)
            {
                error("redefined section privileges:", (void*) truncatedName);
            }
            return &sectionArray[bufferIterator];
        }
    }

    ResolvedSection newSection = {
            .name = truncatedName,
            .characteristics = section->Characteristics,
    };
    initBufferObject(&newSection.buffer);

    return sectionBuffer.bufferPtr + addChunk(&sectionBuffer, &newSection, sizeof(ResolvedSection));
}

/**
 * @param symbol the new symbol to be added
 * @return <b>true</b> if there is a collision
 */
static bool checkSymbolsForCollisions(const char* name)
{
    ResolvedSymbol* symbolArray = symbolBuffer.bufferPtr;
    for (uint32_t i = 0; i < symbolBuffer.currentOffset / sizeof(ResolvedSymbol); i++)
    {
        if (strcmp(name, symbolArray[i].name) == 0)
            return true;
    }

    return false;
}

static void processObjectFile(const char* filePath)
{
    void* file = loadFile(filePath);
    struct COFF_header* coff = file;
    struct COFF_symbol* symbolTable = file + coff->PointerToSymbolTable;
    char* stringTable = (char*) (symbolTable + coff->NumberOfSymbols);

    const uint16_t numberOfSections = coff->NumberOfSections;
    struct Section_header* sections = file + sizeof(struct COFF_header);


    for (uint16_t sectionIterator = 0; sectionIterator < numberOfSections; sectionIterator++)
    {
        struct Section_header* currentSection = &sections[sectionIterator];
        void* rawData = (void*) coff + currentSection->PointerToRawData;
        ResolvedSection* currentResolvedSection = checkExistingSections(currentSection, stringTable);
        uint16_t resolvedSectionIndex = ((void*) currentResolvedSection - sectionBuffer.bufferPtr) / sizeof(ResolvedSection);
        binaryBuffer* buffer = &currentResolvedSection->buffer;
        uint32_t sectionRelocationBase = addChunk(buffer, rawData, currentSection->SizeOfRawData);

        for (uint32_t symbolIterator = 0; symbolIterator < coff->NumberOfSymbols; symbolIterator++)
        {
            struct COFF_symbol symbol = symbolTable[symbolIterator];
            symbolIterator += symbol.NumberOfAuxSymbols;

            if (symbol.StorageClass != 2)
                continue;

            if (symbol.SectionNumber == sectionIterator + 1)    //section table is one-indexed
            {
                char* symbolName = resolveSymbolName(&symbol, stringTable);
                if (checkSymbolsForCollisions(symbolName))
                {
                    error("redefined symbol:", symbolName);
                }
                ResolvedSymbol resSymbol = {
                        .name = symbolName,
                        .address = sectionRelocationBase + symbol.Value,
                        .section = resolvedSectionIndex
                };
                addChunk(&symbolBuffer, &resSymbol, sizeof(ResolvedSymbol));
                //printf("%s, %s, %s\n", filePath, &currentResolvedSection->name, resSymbol.name);
            }
        }

        struct COFF_relocation* relocations = file + currentSection->PointerToRelocations;

        for (uint16_t relocationIterator = 0; relocationIterator < currentSection->NumberOfRelocations; relocationIterator++)
        {
            struct COFF_relocation relocation = relocations[relocationIterator];
            struct COFF_symbol* relocatedSymbol = &symbolTable[relocation.SymbolTableIndex];

            if ((relocatedSymbol->Value == 0 && relocatedSymbol->StorageClass == IMAGE_SYM_CLASS_STATIC) ||
                relocatedSymbol->StorageClass == IMAGE_SYM_CLASS_SECTION)
                continue;

            Relocation internalRelocation = {
                    .name = resolveSymbolName(relocatedSymbol, stringTable),
                    .address = relocation.VirtualAddress,
                    .section = resolvedSectionIndex,
                    .type = relocation.Type
            };
            addChunk(&relocationBuffer, &internalRelocation, sizeof(Relocation));
            //printf("%s %d\n", internalRelocation.name, internalRelocation.address);
        }
    }
}

static void relocateSymbols(void)
{
    for (uint16_t iterator = 0; iterator < (uint16_t) (relocationBuffer.currentOffset / sizeof(Relocation)); iterator++)
    {

    }
}

int main()
{
    initBuffers();
    //processObjectFile("resizeableBuffer.obj");
    processObjectFile("inp.o");
    emitExecutable("out.bin");
    return 0;
}
