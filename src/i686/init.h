#ifndef INIT_H
#define INIT_H

#include "gpt.h"

BOOT_STRUCT(boot_StartupInfo) {
    uint16_t pnpPtr[2];
    int diskNum;
    boot_GPTPartition part;
};

#endif // INIT_H
