#ifndef MEMBIOS_H
#define MEMBIOS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum I686_MemoryMapEntryType {
    I686_AvailableMemory = 1,
    I686_ReservedMemory,
    I686_ACPIReclaimMemory,
    I686_ACPINVSMemory,
};

typedef struct I686_MemoryMapEntry {
    uint64_t startRegion;
    uint64_t regionSize;
    uint32_t type;
    uint32_t flags;
} I686_MemoryMapEntry;

int I686_GetMemoryMap(unsigned* context, I686_MemoryMapEntry* entry);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MEMBIOS_H
