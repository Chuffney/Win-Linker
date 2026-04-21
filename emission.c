#include <emission.h>
#include <globals.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pe.h>
#include <DOS_stub.h>

static uint32_t paddedSize(uint32_t size, uint32_t alignment)
{
    return ((size / alignment) + 1) * alignment;
}

static uint32_t calcBinarySize(void)
{
    uint32_t size = FIXED_HEADER_SIZE;

    uint16_t numberOfSections = sectionBuffer.currentOffset / sizeof(ResolvedSection);
    ResolvedSection* sectionArray = sectionBuffer.bufferPtr;
    size += numberOfSections * SECTION_HEADER_SIZE;

    for (uint16_t sectionIterator = 0; sectionIterator < numberOfSections; sectionIterator++)
    {
        size += sectionArray[sectionIterator].buffer.currentOffset;
    }
    return size;
}

static uint32_t calcHeaderSize(uint32_t alignment)
{
    uint32_t size = FIXED_HEADER_SIZE;

    uint16_t numberOfSections = sectionBuffer.currentOffset / sizeof(ResolvedSection);
    size += numberOfSections * SECTION_HEADER_SIZE;
    return paddedSize(size, alignment);

}

static uint32_t calcImageSize(uint32_t alignment)
{
    uint32_t size = calcHeaderSize(alignment);
    uint16_t numberOfSections = sectionBuffer.currentOffset / sizeof(ResolvedSection);
    ResolvedSection* sectionArray = sectionBuffer.bufferPtr;

    for (uint16_t sectionIterator = 0; sectionIterator < numberOfSections; sectionIterator++)
    {
        size += paddedSize(sectionArray[sectionIterator].buffer.currentOffset, alignment);
    }
    return size;
}

static void outputToFile(const char* fileName, void* buffer, uint32_t size)
{
    FILE* file = fopen(fileName, "wb");

    if (file == NULL)
        error("file could not be created", fileName);

    fwrite(buffer, 1, size, file);
    fclose(file);
}

void emitExecutable(const char* fileName)
{
    const uint32_t PAGE_SIZE = 1024;

    void* outputBuffer = malloc(calcBinarySize());
    memcpy(outputBuffer, DOS_STUB, sizeof(DOS_STUB));
    uint16_t numberOfSections = sectionBuffer.currentOffset / sizeof(ResolvedSection);

    struct COFF_header* coffHeader = outputBuffer + sizeof(DOS_STUB);
    coffHeader->Machine = 0x6486;
    coffHeader->NumberOfSections = numberOfSections;
    coffHeader->TimeDateStamp = 0;  //todo? who cares for that anyway?
    coffHeader->PointerToSymbolTable = 0;
    coffHeader->NumberOfSymbols = 0;
    coffHeader->SizeOfOptionalHeader = sizeof(struct Optional_header);
    coffHeader->Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_RELOCS_STRIPPED | IMAGE_FILE_LARGE_ADDRESS_AWARE;

    struct Optional_header* optionalHeader = (void*) (coffHeader + 1);
    optionalHeader->Magic = PE32_plus;
    optionalHeader->MajorLinkerVersion = 0;
    optionalHeader->MinorLinkerVersion = 0;
    optionalHeader->SizeOfCode = 0; //todo
    optionalHeader->SizeOfInitializedData = 0; //todo
    optionalHeader->SizeOfUninitializedData = 0; //todo
    optionalHeader->AddressOfEntryPoint = 0; //todo
    optionalHeader->BaseOfCode = 0; //todo
    optionalHeader->ImageBase = DEFAULT_IMAGE_BASE;
    optionalHeader->SectionAlignment = PAGE_SIZE;
    optionalHeader->FileAlignment = PAGE_SIZE;
    optionalHeader->MajorOperatingSystemVersion = 0;
    optionalHeader->MinorOperatingSystemVersion = 0;
    optionalHeader->MajorImageVersion = 0;
    optionalHeader->MinorImageVersion = 0;
    optionalHeader->MajorSubsystemVersion = 0;
    optionalHeader->MinorSubsystemVersion = 0;
    optionalHeader->Win32VersionValue = 0;
    optionalHeader->SizeOfImage = calcImageSize(PAGE_SIZE);
    optionalHeader->SizeOfHeaders = calcHeaderSize(PAGE_SIZE);
    optionalHeader->CheckSum = 0;
    optionalHeader->Subsystem = 0;
    optionalHeader->DllCharacteristics = 0;
    optionalHeader->SizeOfStackReserve = 0;
    optionalHeader->SizeOfStackCommit = 0;
    optionalHeader->SizeOfHeapReserve = 0;
    optionalHeader->SizeOfHeapCommit = 0;
    optionalHeader->LoaderFlags = 0;
    optionalHeader->NumberOfRvaAndSizes = 0;

    struct Section_header* sectionHeaderArray = (void*) (optionalHeader + 1);
    ResolvedSection* internalSectionArray = sectionBuffer.bufferPtr;
    uint32_t virtualAddress = DEFAULT_IMAGE_BASE + optionalHeader->SizeOfHeaders;
    for (uint16_t sectionIterator = 0; sectionIterator < numberOfSections; sectionIterator++)
    {
        struct Section_header* sectionHeader = &sectionHeaderArray[sectionIterator];
        ResolvedSection internalSection = internalSectionArray[sectionIterator];

        sectionHeader->Name = internalSection.name;
        sectionHeader->VirtualSize = paddedSize(internalSection.buffer.currentOffset, PAGE_SIZE);
        sectionHeader->VirtualAddress = virtualAddress;
        virtualAddress += sectionHeader->VirtualSize;
        sectionHeader->SizeOfRawData = internalSection.buffer.currentOffset;
        sectionHeader->PointerToRelocations = 0;
        sectionHeader->PointerToLinenumbers = 0;
        sectionHeader->NumberOfRelocations = 0;
        sectionHeader->NumberOfLinenumbers = 0;
        sectionHeader->Characteristics = internalSection.characteristics;
    }

    outputToFile(fileName, outputBuffer, calcBinarySize());
    free(outputBuffer);
}
