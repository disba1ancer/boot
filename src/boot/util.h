#ifndef BT_UTIL_H
#define BT_UTIL_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#define BOOT_STRUCT(name) \
    typedef struct name name; \
    struct name

#define BOOT_TYPEDEF_STRUCT(name) \
    typedef struct name name; \
    typedef struct name

BOOT_STRUCT(boot_DoublyLinkedListElement) {
    boot_DoublyLinkedListElement *next;
    boot_DoublyLinkedListElement *prev;
};

BOOT_STRUCT(boot_DoublyLinkedList) {
    boot_DoublyLinkedListElement *begin;
};

//void DoublyLinkedList_Add(DoublyLinkedList *list, DoublyLinkedListElement *elem);
//void DoublyLinkedList_Remove(DoublyLinkedList *list, DoublyLinkedListElement *elem);

#ifdef __cplusplus
extern "C" {
#endif

inline void boot_DoublyLinkedList_Add(boot_DoublyLinkedList *list, boot_DoublyLinkedListElement *elem)
{
    elem->prev = NULL;
    elem->next = list->begin;
    list->begin = elem;
    if (elem->next != NULL) {
        elem->next->prev = elem;
    }
}

inline void boot_DoublyLinkedList_Remove(boot_DoublyLinkedList *list, boot_DoublyLinkedListElement *elem)
{
    if (elem->prev != NULL) {
        elem->prev->next = elem->next;
    } else {
        list->begin = elem->next;
    }
    if (elem->next != NULL) {
        elem->next->prev = elem->prev;
    }
}

inline int boot_Log2U64(uint64_t val)
{
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_clzll(val) ^ 63;
#else
    int result = 0;
    while (val != 1) {
        val >>= 1;
        ++result;
    }
    return result;
#endif
}

inline int boot_Log2U32(uint32_t val)
{
    return boot_Log2U64(val);
}

#define TMINMAX(s, t)\
    inline t boot_Min ## s(t a, t b)\
    { return (a < b) ? a : b; }\
    inline t boot_Max ## s(t a, t b)\
    { return (a > b) ? a : b; }\
    inline unsigned t boot_MinU ## s(unsigned t a, unsigned t b)\
    { return (a < b) ? a : b; }\
    inline unsigned t boot_MaxU ## s(unsigned t a, unsigned t b)\
    { return (a > b) ? a : b; }

TMINMAX(, int)
TMINMAX(L, long)
TMINMAX(LL, long long)

#undef TMINMAX

#ifndef __cplusplus

#define boot_MinG(s, t)\
    t : boot_Min ## s,\
    unsigned t : boot_MinU ## s

#define boot_Min(a, b) _Generic((a)+(b),\
    boot_MinG(, int,),\
    boot_MinG(L, long),\
    boot_MinG(LL, long long)\
    )(a, b)

#define boot_MaxG(s, t)\
    t : boot_Min ## s,\
    unsigned t : boot_MinU ## s

#define boot_Max(a, b) _Generic((a)+(b),\
    boot_MaxG(, int),\
    boot_MaxG(L, long),\
    boot_MaxG(LL, long long)\
    )(a, b)

#endif

#define TMINMAX(bits)\
    inline uint##bits##_t boot_MinU##bits(uint##bits##_t a, uint##bits##_t b)\
    { return (a < b) ? a : b; }\
    inline uint##bits##_t boot_MaxU##bits(uint##bits##_t a, uint##bits##_t b)\
    { return (a > b) ? a : b; }\
    inline int##bits##_t boot_MinS##bits(int##bits##_t a, int##bits##_t b)\
    { return (a < b) ? a : b; }\
    inline int##bits##_t boot_MaxS##bits(int##bits##_t a, int##bits##_t b)\
    { return (a > b) ? a : b; }

TMINMAX(8)
TMINMAX(16)
TMINMAX(32)
TMINMAX(64)

#undef TMINMAX

#ifdef __cplusplus
} // extern "C"

namespace boot {

#define NS_FUNC(prefix, name) inline constexpr auto& name = :: prefix ## name;

NS_FUNC(boot_, DoublyLinkedList_Add)
NS_FUNC(boot_, DoublyLinkedList_Remove)

template <typename A, typename B>
constexpr auto Min(const A& a, const B& b) -> decltype(a + b) {
    return (a < b) ? a : b;
}

template <typename A, typename B>
constexpr auto Max(const A& a, const B& b) -> decltype(a + b) {
    return (a > b) ? a : b;
}

#define TMINMAX(bits)\
    NS_FUNC(boot_, MinU##bits)\
    NS_FUNC(boot_, MaxU##bits)\
    NS_FUNC(boot_, MinS##bits)\
    NS_FUNC(boot_, MaxS##bits)

TMINMAX(8)
TMINMAX(16)
TMINMAX(32)
TMINMAX(64)

#undef TMINMAX

#undef NS_FUNC

}

#endif

#endif // BT_UTIL_H
