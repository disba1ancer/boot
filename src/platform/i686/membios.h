#ifndef MEMBIOS_H
#define MEMBIOS_H

#include <stdint.h>
#include "boot/util.h"
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

enum i686_bios_mem_MapEntryType {
    i686_bios_mem_AvailableMemory = 1,
    i686_bios_mem_ReservedMemory,
    i686_bios_mem_ACPIReclaimMemory,
    i686_bios_mem_ACPINVSMemory,
};

BOOT_STRUCT(i686_bios_mem_MemoryMapEntry) {
    alignas(8)
    uint64_t begin;
    uint64_t size;
    uint32_t type;
    uint32_t flags;
};

int i686_bios_mem_GetMap(unsigned* context, i686_bios_mem_MemoryMapEntry* entry);

#ifdef __cplusplus
} // extern "C"

namespace i686::bios::mem {

//using MapEntry = i686_bios_mem_MapEntry;
using MapEntryType = i686_bios_mem_MapEntryType;

using MemoryMapEntry = i686_bios_mem_MemoryMapEntry;

inline constexpr auto MapEntryType_AvailableMemory = i686_bios_mem_AvailableMemory;
inline constexpr auto MapEntryType_ReservedMemory = i686_bios_mem_ReservedMemory;
inline constexpr auto MapEntryType_ACPIReclaimMemory = i686_bios_mem_ACPIReclaimMemory;
inline constexpr auto MapEntryType_ACPINVSMemory = i686_bios_mem_ACPINVSMemory;

inline constexpr auto &GetMap = i686_bios_mem_GetMap;

}

#endif

#endif // MEMBIOS_H
