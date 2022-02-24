#include "util.h"

extern inline void boot_DoublyLinkedList_Add(boot_DoublyLinkedList *, boot_DoublyLinkedListElement *);
extern inline void boot_DoublyLinkedList_Remove(boot_DoublyLinkedList *, boot_DoublyLinkedListElement *);

extern inline int  boot_Log2U64(uint64_t val);
extern inline int  boot_Log2U32(uint32_t val);

#undef boot_Min
#undef boot_Max

#define TMINMAX(s, t)\
    extern inline t boot_Min ## s(t a, t b);\
    extern inline t boot_Max ## s(t a, t b);\
    extern inline unsigned t boot_MinU ## s(unsigned t a, unsigned t b);\
    extern inline unsigned t boot_MaxU ## s(unsigned t a, unsigned t b);

TMINMAX(, int)
TMINMAX(L, long)
TMINMAX(LL, long long)

#undef TMINMAX

#define TMINMAX(bits)\
    extern inline uint ## bits ## _t boot_MinU ## bits(uint ## bits ## _t a, uint ## bits ## _t b);\
    extern inline uint ## bits ## _t boot_MaxU ## bits(uint ## bits ## _t a, uint ## bits ## _t b);\
    extern inline  int ## bits ## _t boot_MinS ## bits( int ## bits ## _t a,  int ## bits ## _t b);\
    extern inline  int ## bits ## _t boot_MaxS ## bits( int ## bits ## _t a,  int ## bits ## _t b);

TMINMAX(8)
TMINMAX(16)
TMINMAX(32)
TMINMAX(64)

#undef TMINMAX
