#include "boot/endian.h"

extern inline uint16_t boot_LE16ULoad(const boot_LE16U* val);
extern inline uint32_t boot_LE32ULoad(const boot_LE32U* val);
extern inline uint64_t boot_LE64ULoad(const boot_LE64U* val);
extern inline  int16_t boot_LE16SLoad(const boot_LE16S* val);
extern inline  int32_t boot_LE32SLoad(const boot_LE32S* val);
extern inline  int64_t boot_LE64SLoad(const boot_LE64S* val);
extern inline uint16_t boot_BE16ULoad(const boot_BE16U* val);
extern inline uint32_t boot_BE32ULoad(const boot_BE32U* val);
extern inline uint64_t boot_BE64ULoad(const boot_BE64U* val);
extern inline  int16_t boot_BE16SLoad(const boot_BE16S* val);
extern inline  int32_t boot_BE32SLoad(const boot_BE32S* val);
extern inline  int64_t boot_BE64SLoad(const boot_BE64S* val);
extern inline uint16_t boot_RE16ULoad(const boot_RE16U* val, int endian);
extern inline uint32_t boot_RE32ULoad(const boot_RE32U* val, int endian);
extern inline uint64_t boot_RE64ULoad(const boot_RE64U* val, int endian);
extern inline  int16_t boot_RE16SLoad(const boot_RE16S* val, int endian);
extern inline  int32_t boot_RE32SLoad(const boot_RE32S* val, int endian);
extern inline  int64_t boot_RE64SLoad(const boot_RE64S* val, int endian);
