#ifndef LINKER_EMISSION_H
#define LINKER_EMISSION_H

#define FIXED_HEADER_SIZE 0x80 + 0x14 + 0x58
#define SECTION_HEADER_SIZE 40


void emitExecutable(const char* fileName);

#endif //LINKER_EMISSION_H
