#ifndef PE_H
#define PE_H

#include <stdint.h>

/**
 * Official Microsoft source:
 * https://learn.microsoft.com/en-gb/windows/win32/debug/pe-format
 */

#define FILE_OFFSET 0x3c    //where PE_SIGNATURE is located
#define PE_SIGNATURE "PE\0\0"
#define SYMBOL_FUNCTION 0x20

struct COFF_header
{
    /**
     * The number that identifies the type of target machine. For more information, see Machine Types.
     */
    uint16_t Machine;
    /**
     * The number of sections. This indicates the size of the section table, which immediately follows the headers.
     */
    uint16_t NumberOfSections;
    /**
     * The low 32 bits of the number of seconds since 00:00 January 1, 1970 (a C run-time time_t value), which indicates when the file was created.
     */
    uint32_t TimeDateStamp;
    /**
     * The file offset of the COFF symbol table, or zero if no COFF symbol table is present. This value should be zero for an image because COFF debugging information is deprecated.
     */
    uint32_t PointerToSymbolTable;
    /**
     * The number of entries in the symbol table. This data can be used to locate the string table, which immediately follows the symbol table. This value should be zero for an image because COFF debugging information is deprecated.
     */
    uint32_t NumberOfSymbols;
    /**
     * The size of the optional header, which is required for executable files but not for object files. This value should be zero for an object file. For a description of the header format, see Optional Header (Image Only).
     */
    uint16_t SizeOfOptionalHeader;
    /**
     * The flags that indicate the attributes of the file. For specific flag values, see Characteristics.
     */
    uint16_t Characteristics;
};

struct Section_header
{
    /**
     * An 8-byte, null-padded UTF-8 encoded string. If the string is exactly 8 characters long, there is no terminating null. For longer names, this field contains a slash (/) that is followed by an ASCII representation of a decimal number that is an offset into the string table. Executable images do not use a string table and do not support section names longer than 8 characters. Long names in object files are truncated if they are emitted to an executable file.
     */
    uint64_t Name;
    /**
     * The total size of the section when loaded into memory. If this value is greater than SizeOfRawData, the section is zero-padded. This field is valid only for executable images and should be set to zero for object files.
     */
    uint32_t VirtualSize;
    /**
     * For executable images, the address of the first byte of the section relative to the image base when the section is loaded into memory. For object files, this field is the address of the first byte before relocation is applied; for simplicity, compilers should set this to zero. Otherwise, it is an arbitrary value that is subtracted from offsets during relocation.
     */
    uint32_t VirtualAddress;
    /**
     * The size of the section (for object files) or the size of the initialized data on disk (for image files). For executable images, this must be a multiple of FileAlignment from the optional header. If this is less than VirtualSize, the remainder of the section is zero-filled. Because the SizeOfRawData field is rounded but the VirtualSize field is not, it is possible for SizeOfRawData to be greater than VirtualSize as well. When a section contains only uninitialized data, this field should be zero.
     */
    uint32_t SizeOfRawData;
    /**
     * The file pointer to the first page of the section within the COFF file. For executable images, this must be a multiple of FileAlignment from the optional header. For object files, the value should be aligned on a 4-byte boundary for best performance. When a section contains only uninitialized data, this field should be zero.
     */
    uint32_t PointerToRawData;
    /**
     * The file pointer to the beginning of relocation entries for the section. This is set to zero for executable images or if there are no relocations.
     */
    uint32_t PointerToRelocations;
    /**
     * The file pointer to the beginning of line-number entries for the section. This is set to zero if there are no COFF line numbers. This value should be zero for an image because COFF debugging information is deprecated.
     */
    uint32_t PointerToLinenumbers;
    /**
     * The number of relocation entries for the section. This is set to zero for executable images.
     */
    uint16_t NumberOfRelocations;
    /**
     * The number of line-number entries for the section. This value should be zero for an image because COFF debugging information is deprecated.
     */
    uint16_t NumberOfLinenumbers;
    /**
     * The flags that describe the characteristics of the section. For more information, see Section Flags.
     */
    uint32_t Characteristics;
};

#pragma pack(push, 1)
struct SymbolTableEntry //18 bytes
{
    union
    {
        uint8_t ShortName[8];
        struct
        {
            uint32_t Zeroes;
            uint32_t Offset;
        };
    };
    uint32_t Value;
    uint16_t SectionNumber;
    uint16_t Type;
    uint8_t StorageClass;
    uint8_t NumberOfAuxSymbols;
};
#pragma pack(pop)


//Characteristics
#define IMAGE_SCN_MEM_EXECUTE 0x20000000
#define IMAGE_SCN_MEM_READ 0x40000000
#define IMAGE_SCN_MEM_WRITE 0x80000000

#endif
