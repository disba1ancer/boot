#ifndef DATA_H
#define DATA_H

#include "util.h"
#include <stdalign.h>

BOOT_STRUCT(boot_LdrData) {
    alignas(8)
    uint64_t type;
    uint64_t value;
};

enum boot_LdrDataType {
    boot_LdrDataType_EntriesCount = 0,
    boot_LdrDataType_MemoryMap = 1,
    boot_LdrDataType_VideoMode = 2,
    boot_LdrDataType_DriveParams = 3,
};

enum boot_MemoryMapEntryType {
    boot_MemoryMapEntryType_AvailableMemory,
    boot_MemoryMapEntryType_BootReclaimable,
    boot_MemoryMapEntryType_SystemReclaimable,
    boot_MemoryMapEntryType_ReservedMemory,
};

BOOT_STRUCT(boot_MemoryMapEntry) {
    alignas(8)
    uint64_t begin;
    uint64_t end;
    uint32_t type;
};

BOOT_STRUCT(boot_MemoryMap) {
    alignas(8)
    uint64_t entries;
    uint64_t count;
};

#endif // DATA_H
