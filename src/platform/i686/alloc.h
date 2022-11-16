#ifndef ALLOC_H
#define ALLOC_H

#include "boot/util.h"
#include "membios.h"

void boot_InitBuddyAlloc();
void boot_InitVirtualAlloc();

extern unsigned char __bss_end[];

#endif // ALLOC_H
