#ifndef VIRTUAL_ALLOC_H
#define VIRTUAL_ALLOC_H

#include <stddef.h>
#include <stdint.h>

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

void* boot_AllocPage(void);
void* boot_VirtualAlloc(uint64_t virtPageAddr, int flags);
void boot_VirtualEnter(uint64_t entryPoint);

#ifdef __cplusplus
}
#endif

#endif // VIRTUAL_ALLOC_H
