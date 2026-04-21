//doc: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format

#ifndef PE_H
#define PE_H

#include <stdint.h>

#pragma pack(push, 1)

#define PE_SIGNATURE_OFFSET 0x3c    //where offset to the PE_SIGNATURE is located
#define OPTIONAL_HEADER_SIZE 0x58


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

//file Characteristics
#define IMAGE_FILE_RELOCS_STRIPPED  0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE 0x0002
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020

struct Optional_header  //PE32+
{
    /**
     * The unsigned integer that identifies the state of the image file. The most common number is 0x10B, which identifies it as a normal executable file. 0x107 identifies it as a ROM image, and 0x20B identifies it as a PE32+ executable.
     */
    uint16_t Magic;
    /**
     * The linker major version number.
     */
    uint8_t MajorLinkerVersion;
    /**
     * The linker minor version number.
     */
    uint8_t MinorLinkerVersion;
    /**
     * The size of the code (text) section, or the sum of all code sections if there are multiple sections.
     */
    uint32_t SizeOfCode;
    /**
     * The size of the initialized data section, or the sum of all such sections if there are multiple data sections.
     */
    uint32_t SizeOfInitializedData;
    /**
     * The size of the uninitialized data section (BSS), or the sum of all such sections if there are multiple BSS sections.
     */
    uint32_t SizeOfUninitializedData;
    /**
     * The address of the entry point relative to the image base when the executable file is loaded into memory. For program images, this is the starting address. For device drivers, this is the address of the initialization function. An entry point is optional for DLLs. When no entry point is present, this field must be zero.
     */
    uint32_t AddressOfEntryPoint;
    /**
     * The address that is relative to the image base of the beginning-of-code section when it is loaded into memory.
     */
    uint32_t BaseOfCode;
    /**
     * The preferred address of the first byte of image when loaded into memory; must be a multiple of 64 K. The default for DLLs is 0x10000000. The default for Windows CE EXEs is 0x00010000. The default for Windows NT, Windows 2000, Windows XP, Windows 95, Windows 98, and Windows Me is 0x00400000.
     */
    uint64_t ImageBase;
    /**
     * The alignment (in bytes) of sections when they are loaded into memory. It must be greater than or equal to FileAlignment. The default is the page size for the architecture.
     */
    uint32_t SectionAlignment;
    /**
     * The alignment factor (in bytes) that is used to align the raw data of sections in the image file. The value should be a power of 2 between 512 and 64 K, inclusive. The default is 512. If the SectionAlignment is less than the architecture's page size, then FileAlignment must match SectionAlignment.
     */
    uint32_t FileAlignment;

    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    /**
     * Reserved, must be zero.
     */
    uint32_t Win32VersionValue;
    /**
     * The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
     */
    uint32_t SizeOfImage;
    /**
     * The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
     */
    uint32_t SizeOfHeaders;
    /**
     * The image file checksum. The algorithm for computing the checksum is incorporated into IMAGHELP.DLL. The following are checked for validation at load time: all drivers, any DLL loaded at boot time, and any DLL that is loaded into a critical Windows process.
     */
    uint32_t CheckSum;
    /**
     * The subsystem that is required to run this image. For more information, see Windows Subsystem.
     */
    uint16_t Subsystem;
    /**
     * For more information, see DLL Characteristics later in this specification.
     */
    uint16_t DllCharacteristics;
    /**
     * The size of the stack to reserve. Only SizeOfStackCommit is committed; the rest is made available one page at a time until the reserve size is reached.
     */
    uint64_t SizeOfStackReserve;
    /**
     * The size of the stack to commit.
     */
    uint64_t SizeOfStackCommit;
    /**
     * The size of the local heap space to reserve. Only SizeOfHeapCommit is committed; the rest is made available one page at a time until the reserve size is reached.
     */
    uint64_t SizeOfHeapReserve;
    /**
     * The size of the local heap space to commit.
     */
    uint64_t SizeOfHeapCommit;
    /**
     * Reserved, must be zero.
     */
    uint32_t LoaderFlags;
    /**
     * The number of data-directory entries in the remainder of the optional header. Each describes a location and size.
     */
    uint32_t NumberOfRvaAndSizes;
};

#define PE32_plus 0x20B
#define DEFAULT_IMAGE_BASE 0x00400000

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

//Characteristics
#define IMAGE_SCN_MEM_EXECUTE   0x20000000
#define IMAGE_SCN_MEM_READ      0x40000000
#define IMAGE_SCN_MEM_WRITE     0x80000000

struct COFF_symbol
{
    /**
     * The name of the symbol, represented by a union of three structures. An array of 8 bytes is used if the name is not more than 8 bytes long.
     */
    union
    {
        /**
         * An array of 8 bytes. This array is padded with nulls on the right if the name is less than 8 bytes long.
         */
        uint64_t ShortName;
        struct
        {
            /**
             * A field that is set to all zeros if the name is longer than 8 bytes.
             */
            uint32_t Zeroes;
            /**
             * An offset into the string table.
             */
            uint32_t Offset;
        };
    } Name;
    /**
     * The value that is associated with the symbol. The interpretation of this field depends on SectionNumber and StorageClass. A typical meaning is the relocatable address.
     */
    uint32_t Value;
    /**
     * The signed integer that identifies the section, using a one-based index into the section table. Some values have special meaning, as defined in section 5.4.2, "Section Number Values."
     */
    int16_t SectionNumber;
    /**
     * A number that represents type. Microsoft tools set this field to 0x20 (function) or 0x0 (not a function).
     */
    uint16_t Type;
    /**
     * An enumerated value that represents storage class.
     */
    uint8_t StorageClass;
    /**
     * The number of auxiliary symbol table entries that follow this record.
     */
    uint8_t NumberOfAuxSymbols;
};

//StorageClass
#define IMAGE_SYM_CLASS_STATIC  3
#define IMAGE_SYM_CLASS_SECTION 104

struct COFF_relocation
{
    /**
     * The address of the item to which relocation is applied. This is the offset from the beginning of the section, plus the value of the section's RVA/Offset field.
     */
    uint32_t VirtualAddress;
    /**
     * A zero-based index into the symbol table. This symbol gives the address that is to be used for the relocation. If the specified symbol has section storage class, then the symbol's address is the address with the first section of the same name.
     */
    uint32_t SymbolTableIndex;
    /**
     * A value that indicates the kind of relocation that should be performed.
     */
    uint16_t Type;
};

//relocation types
#define IMAGE_REL_AMD64_ABSOLUTE    0x0000
#define IMAGE_REL_AMD64_REL32       0x0004

#pragma pack(pop)

#endif  //PE_H
