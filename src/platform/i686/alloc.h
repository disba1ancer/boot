#ifndef ALLOC_H
#define ALLOC_H

#include "boot/data.h"

enum Constants {
    boot_Alloc_PageSize = 0x1000,
};

void boot_InitAlloc(void);
void boot_InitVirtualAlloc(boot_MemoryMap* memmap);

#endif // ALLOC_H
