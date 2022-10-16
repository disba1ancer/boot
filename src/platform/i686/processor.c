#include "processor.h"

extern inline uint64_t x86_64_PageEntry_GetAddr(const x86_64_PageEntry* entry);
extern inline uint32_t x86_64_PageEntry_GetFlags(const x86_64_PageEntry* entry);
extern inline void *i686_LoadPointer(i686_RMPtr fptr);
extern inline i686_RMPtr i686_MakeRMPointer(void *ptr);
