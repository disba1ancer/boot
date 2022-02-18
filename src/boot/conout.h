#ifndef CONOUT_H
#define CONOUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum boot_conout_constants {
    boot_conout_FromBegin,
    boot_conout_FromCurrent
};

void boot_conout_PutC(char c);
void boot_conout_Write(const char *buf, size_t size);
void boot_conout_PutStr(const char *str);
void boot_conout_Seek(size_t pos, int origin);
size_t boot_conout_Tell();
size_t boot_conout_ChPerLine();

#ifdef __cplusplus
} // extern "C"

namespace boot::conout {
inline constexpr auto PutC = boot_conout_PutC;
inline constexpr auto Write = boot_conout_Write;
inline constexpr auto PutStr = boot_conout_PutStr;
}

#endif

#endif // CONOUT_H
