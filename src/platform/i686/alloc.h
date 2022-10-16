#ifndef ALLOC_H
#define ALLOC_H

#include "boot/util.h"
#include "membios.h"

BOOT_STRUCT(i686_mem_entries) {
    size_t count;
    i686_bios_mem_MapEntry entries[];
};

void boot_InitBuddyAlloc();
void boot_InitVirtualAlloc();

extern unsigned char __bss_end[];

#endif // ALLOC_H
