#include "alloc.h"
#include "init.h"
#include <stdlib.h>

void _init(void);

void boot_main(boot_StartupInfo *si);

_Noreturn void c_start(boot_StartupInfo si) {
    boot_InitBuddyAlloc();
    _init();
    boot_main(&si);
    exit(0);
}
