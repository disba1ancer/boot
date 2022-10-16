#ifndef ENDIAN_H
#define ENDIAN_H

#include "util.h"

BOOT_STRUCT(boot_LE16S) {
    unsigned char val[2];
};

BOOT_STRUCT(boot_LE32S) {
    unsigned char val[4];
};

BOOT_STRUCT(boot_LE64S) {
    unsigned char val[8];
};

BOOT_STRUCT(boot_LE16U) {
    unsigned char val[2];
};

BOOT_STRUCT(boot_LE32U) {
    unsigned char val[4];
};

BOOT_STRUCT(boot_LE64U) {
    unsigned char val[8];
};

BOOT_STRUCT(boot_BE16S) {
    unsigned char val[2];
};

BOOT_STRUCT(boot_BE32S) {
    unsigned char val[4];
};

BOOT_STRUCT(boot_BE64S) {
    unsigned char val[8];
};

BOOT_STRUCT(boot_BE16U) {
    unsigned char val[2];
};

BOOT_STRUCT(boot_BE32U) {
    unsigned char val[4];
};

BOOT_STRUCT(boot_BE64U) {
    unsigned char val[8];
};

BOOT_UNION(boot_RE16S) {
    boot_LE16S l;
    boot_BE16S b;
};

BOOT_UNION(boot_RE32S) {
    boot_LE32S l;
    boot_BE32S b;
};

BOOT_UNION(boot_RE64S) {
    boot_LE64S l;
    boot_BE64S b;
};

BOOT_UNION(boot_RE16U) {
    boot_LE16U l;
    boot_BE16U b;
};

BOOT_UNION(boot_RE32U) {
    boot_LE32U l;
    boot_BE32U b;
};

BOOT_UNION(boot_RE64U) {
    boot_LE64U l;
    boot_BE64U b;
};

#define BOOT_ENDIAN_BSWAP {\
    typedef unsigned char uchar;\
    uchar* data = (uchar*)&val;\
    for (size_t i = 0; i < (sizeof(val) / 2); ++i) {\
        uchar tmp = data[i];\
        data[i] = data[sizeof(val) - i - 1];\
        data[sizeof(val) - i - 1] = tmp;\
    }\
    return val;\
}

#ifdef BOOT_BIG_ENDIAN_HOST
#define BOOT_LOAD_STORE_BE { return val; }
#define BOOT_LOAD_STORE_LE BOOT_ENDIAN_BSWAP
#else
#define BOOT_LOAD_STORE_BE BOOT_ENDIAN_BSWAP
#define BOOT_LOAD_STORE_LE { return val; }
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BOOT_BIG_ENDIAN_HOST
inline uint16_t boot_BE16ULoad(const boot_BE16U* val)
#else
inline uint16_t boot_LE16ULoad(const boot_LE16U* val)
#endif
{
    return
        (uint16_t)val->val[0] |
        (uint16_t)((uint16_t)val->val[1] << 8);
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline uint16_t boot_BE32ULoad(boot_BE32U* val)
#else
inline uint32_t boot_LE32ULoad(const boot_LE32U* val)
#endif
{
    return
        (uint32_t)val->val[0] |
        (uint32_t)val->val[1] << 8U |
        (uint32_t)val->val[2] << 16U |
        (uint32_t)val->val[3] << 24U;
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline uint16_t boot_BE64ULoad(const boot_BE64U* val)
#else
inline uint64_t boot_LE64ULoad(const boot_LE64U* val)
#endif
{
    return
        (uint64_t)val->val[0] |
        (uint64_t)val->val[1] << 8U |
        (uint64_t)val->val[2] << 16U |
        (uint64_t)val->val[3] << 24U |
        (uint64_t)val->val[4] << 32U |
        (uint64_t)val->val[5] << 40U |
        (uint64_t)val->val[6] << 48U |
        (uint64_t)val->val[7] << 56U;
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline int16_t boot_BE16SLoad(const boot_BE16S* val)
#else
inline int16_t boot_LE16SLoad(const boot_LE16S* val)
#endif
{
    return
        (int16_t)val->val[0] |
        (int16_t)((int16_t)val->val[1] << 8);
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline int16_t boot_BE32SLoad(const boot_BE32S* val)
#else
inline int32_t boot_LE32SLoad(const boot_LE32S* val)
#endif
{
    return
        (int32_t)val->val[0] |
        (int32_t)val->val[1] << 8U |
        (int32_t)val->val[2] << 16U |
        (int32_t)val->val[3] << 24U;
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline int16_t boot_BE64SLoad(const boot_BE64S* val)
#else
inline int64_t boot_LE64SLoad(const boot_LE64S* val)
#endif
{
    return
        (int64_t)val->val[0] |
        (int64_t)val->val[1] << 8U |
        (int64_t)val->val[2] << 16U |
        (int64_t)val->val[3] << 24U |
        (int64_t)val->val[4] << 32U |
        (int64_t)val->val[5] << 40U |
        (int64_t)val->val[6] << 48U |
        (int64_t)val->val[7] << 56U;
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline uint16_t boot_LE16ULoad(const boot_LE16U* val)
#else
inline uint16_t boot_BE16ULoad(const boot_BE16U* val)
#endif
{
    return
        (uint16_t)val->val[1] |
        (uint16_t)((uint16_t)val->val[0] << 8);
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline uint32_t boot_LE32ULoad(const boot_LE32U* val)
#else
inline uint32_t boot_BE32ULoad(const boot_BE32U* val)
#endif
{
    return
        (uint32_t)val->val[3] |
        (uint32_t)val->val[2] << 8U |
        (uint32_t)val->val[1] << 16U |
        (uint32_t)val->val[0] << 24U;
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline uint64_t boot_LE64ULoad(const boot_LE64U* val)
#else
inline uint64_t boot_BE64ULoad(const boot_BE64U* val)
#endif
{
    return
        (uint64_t)val->val[7] |
        (uint64_t)val->val[6] << 8U |
        (uint64_t)val->val[5] << 16U |
        (uint64_t)val->val[4] << 24U |
        (uint64_t)val->val[3] << 32U |
        (uint64_t)val->val[2] << 40U |
        (uint64_t)val->val[1] << 48U |
        (uint64_t)val->val[0] << 56U;
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline int16_t boot_LE16SLoad(const boot_LE16S* val)
#else
inline int16_t boot_BE16SLoad(const boot_BE16S* val)
#endif
{
    return
        (int16_t)val->val[1] |
        (int16_t)((int16_t)val->val[0] << 8);
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline int32_t boot_LE32SLoad(const boot_LE32S* val)
#else
inline int32_t boot_BE32SLoad(const boot_BE32S* val)
#endif
{
    return
        (int32_t)val->val[3] |
        (int32_t)val->val[2] << 8U |
        (int32_t)val->val[1] << 16U |
        (int32_t)val->val[0] << 24U;
}

#ifdef BOOT_BIG_ENDIAN_HOST
inline int64_t boot_LE64SLoad(const boot_LE64S* val)
#else
inline int64_t boot_BE64SLoad(const boot_BE64S* val)
#endif
{
    return
        (int64_t)val->val[7] |
        (int64_t)val->val[6] << 8U |
        (int64_t)val->val[5] << 16U |
        (int64_t)val->val[4] << 24U |
        (int64_t)val->val[3] << 32U |
        (int64_t)val->val[2] << 40U |
        (int64_t)val->val[1] << 48U |
        (int64_t)val->val[0] << 56U;
}

enum boot_Endian {
    boot_Endian_Little,
    boot_Endian_Big
};

#define GENERATE(bit)\
inline int##bit##_t boot_RE##bit##SLoad(const boot_RE##bit##S* val, int endian) {\
    if (endian == boot_Endian_Little) {\
        return boot_LE##bit##SLoad(&val->l);\
    } else {\
        return boot_BE##bit##SLoad(&val->b);\
    }\
}\
inline uint##bit##_t boot_RE##bit##ULoad(const boot_RE##bit##U* val, int endian) {\
    if (endian == boot_Endian_Little) {\
        return boot_LE##bit##ULoad(&val->l);\
    } else {\
        return boot_BE##bit##ULoad(&val->b);\
    }\
}

GENERATE(16)
GENERATE(32)
GENERATE(64)

#undef GENERATE

#ifdef __cplusplus
} //extern "C"

namespace boot {

using Endian = ::boot_Endian;
inline constexpr auto Endian_Little = ::boot_Endian_Little;
inline constexpr auto Endian_Big =  ::boot_Endian_Big;

#define GENERATE(bit)\
inline uint##bit##_t ELoad(const boot_LE##bit##U& val) {\
    return boot_LE##bit##ULoad(&val);\
}\
inline int##bit##_t ELoad(const boot_LE##bit##S& val) {\
    return boot_LE##bit##SLoad(&val);\
}\
inline uint##bit##_t ELoad(const boot_BE##bit##U& val) {\
    return boot_BE##bit##ULoad(&val);\
}\
inline int##bit##_t ELoad(const boot_BE##bit##S& val) {\
    return boot_BE##bit##SLoad(&val);\
}\
inline uint##bit##_t ELoad(const boot_RE##bit##U& val, Endian en) {\
    return boot_RE##bit##ULoad(&val, en);\
}\
inline int##bit##_t ELoad(const boot_RE##bit##S& val, Endian en) {\
    return boot_RE##bit##SLoad(&val, en);\
}

GENERATE(16)
GENERATE(32)
GENERATE(64)

#undef GENERATE

} // namespace boot

#endif

#endif // ENDIAN_H
