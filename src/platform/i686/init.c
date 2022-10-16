#include "alloc.h"
#include "init.h"
#include <stdlib.h>

extern unsigned char __bss_end[];

void _init(void);

void boot_main(boot_StartupInfo *si, size_t size, i686_bios_mem_MapEntry *memmap);

_Noreturn void c_start(boot_StartupInfo si) {
    boot_InitBuddyAlloc();
    _init();
    i686_mem_entries* entries = (void*)__bss_end;
    boot_main(&si, entries->count, entries->entries);
    exit(0);
}
