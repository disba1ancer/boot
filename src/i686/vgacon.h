#ifndef VGACON_H
#define VGACON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile uint16_t i686_vga_console[0x8000 / sizeof(uint16_t)];

#ifdef __cplusplus
} // extern "C"

namespace i686::vga {

inline auto& console = i686_vga_console;

}

#endif

#endif // VGACON_H
