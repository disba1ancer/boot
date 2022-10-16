#ifndef ELF64_H
#define ELF64_H

#include "boot/util.h"
#include "boot/endian.h"

BOOT_STRUCT(elf_ident) {
    byte magic[4];
    byte fileClass;
    byte data;
    byte version;
    byte osABI;
    byte abiVersion;
    byte pad[7];
};

BOOT_STRUCT(elf64_Header) {
    boot_RE16U type;
    boot_RE16U machine;
    boot_RE32U version;
    boot_RE64U entry;
    boot_RE64U programHeaderOff;
    boot_RE64U sectionHeaderOff;
    boot_RE32U flags;
    boot_RE16U headerSize;
    boot_RE16U progHeadEntrySize;
    boot_RE16U progHeadEntryCount;
    boot_RE16U sectHeadEntrySize;
    boot_RE16U sectHeadEntryCount;
    boot_RE16U stringSectionIndex;
};

enum elf_Class {
    elf_Class_C32 = 1,
    elf_Class_C64,
};

enum elf_DataEnc {
    elf_DataEnc_LSB = 1,
    elf_DataEnc_MSB,
};

enum elf_OSABI {
    elf_OSABI_SysV,
    elf_OSABI_HPUX,
    elf_OSABI_Standalone = 255,
};

enum elf_Type {
    elf_Type_None,
    elf_Type_Relocatable,
    elf_Type_Executable,
    elf_Type_Dynamic,
    elf_Type_Core,
    elf_Type_LoOS = 0xFE00,
    elf_Type_HiOS = 0xFEFF,
    elf_Type_LoProc = 0xFF00,
    elf_Type_HiProc = 0xFFFF,
};

enum elf_Machine {
    elf_Machine_None,
    elf_Machine_M32,
    elf_Machine_SPARC,
    elf_Machine_386,
    elf_Machine_68K,
    elf_Machine_88K,
    elf_Machine_860,
    elf_Machine_MIPS,
    elf_Machine_x86_64 = 62,
};

BOOT_STRUCT(elf64_ProgramHeader) {
    boot_RE32U type;
    boot_RE32U flags;
    boot_RE64U offset;
    boot_RE64U virtualAddress;
    boot_RE64U PAddress;
    boot_RE64U size;
    boot_RE64U loadSize;
    boot_RE64U align;
};

enum elf64_SegType {
    elf64_SegType_Null,
    elf64_SegType_Load,
    elf64_SegType_Dynamic,
    elf64_SegType_Interp,
    elf64_SegType_Note,
    elf64_SegType_SHLib,
    elf64_SegType_ProgramHeader,
    elf64_SegType_LoOS = 0x60000000,
    elf64_SegType_HiOS = 0x6FFFFFFF,
    elf64_SegType_LoProc = 0x70000000,
    elf64_SegType_HiProc = 0x7FFFFFFF,
};

enum elf64_SegFlags {
    elf64_SegFlags_X = 1,
    elf64_SegFlags_W = 2,
    elf64_SegFlags_R = 4,
    elf64_SegFlags_MaskOS = 0xFF0000,
    elf64_SegFlags_ProcOS = 0xFF000000,
};

#endif // ELF64_H
