#ifndef MEMBIOS_H
#define MEMBIOS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum i686_MemoryMapEntryType {
    I686_AvailableMemory = 1,
    I686_ReservedMemory,
    I686_ACPIReclaimMemory,
    I686_ACPINVSMemory,
};

typedef struct i686_MemoryMapEntry {
    uint64_t startRegion;
    uint64_t regionSize;
    uint32_t type;
    uint32_t flags;
} i686_MemoryMapEntry;

int i686_GetMemoryMap(unsigned* context, i686_MemoryMapEntry* entry);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MEMBIOS_H
