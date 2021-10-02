#ifndef BIOS_KEYBOARD_H
#define BIOS_KEYBOARD_H

#include <stdint.h>
#include "boot/util.h"

BOOT_STRUCT(i686_bios_kbrd_Key) {
    char ascii;
    uint8_t scanCode;
};

#ifdef __cplusplus
extern "C" {
#endif

i686_bios_kbrd_Key i686_bios_kbrd_GetKey();

#ifdef __cplusplus
}

namespace i686::bios::kbrd {

using Key = i686_bios_kbrd_Key;

inline Key GetKey() { return i686_bios_kbrd_GetKey(); }

}

#endif

#endif // BIOS_KEYBOARD_H
