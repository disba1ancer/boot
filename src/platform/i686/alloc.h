#ifndef ALLOC_H
#define ALLOC_H

#include "boot/util.h"
#include "membios.h"

void boot_InitBuddyAlloc(void);
void boot_InitVirtualAlloc(void);

extern unsigned char __bss_end[];

#endif // ALLOC_H
