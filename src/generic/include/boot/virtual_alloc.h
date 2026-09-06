#ifndef VIRTUAL_ALLOC_H
#define VIRTUAL_ALLOC_H

#include <stddef.h>
#include <stdint.h>
#include "boot/data.h"

#ifdef __cplusplus
extern "C" {
#endif

enum boot_MemoryFlags {
    boot_MemoryFlags_Execute = 1,
    boot_MemoryFlags_Write,
    boot_MemoryFlags_Read = 4,
    boot_MemoryFlags_Kernel = 8,
    boot_MemoryFlags_DeviceWrite = 16,
    boot_MemoryFlags_Device = 32,
};

uint64_t boot_GetAllocCurrentAddr(void);
void* boot_AllocPage(void);
void* boot_VirtualAlloc(uint64_t virtPageAddr, int flags);
int boot_VirtualMap(uint64_t virtPageAddr, uint64_t phyPageAddr, int flags);
void boot_VirtualEnter(uint64_t entryPoint);
void boot_CommitKernelMemory(void);


extern boot_MemoryMap* boot_memmap;

#ifdef __cplusplus
}
#endif

#endif // VIRTUAL_ALLOC_H
