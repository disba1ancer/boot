#ifndef BT_UTIL_H
#define BT_UTIL_H

#define BOOT_STRUCT(name) \
    typedef struct name name; \
    struct name

#define BOOT_TYPEDEF_STRUCT(name) \
    typedef struct name name; \
    typedef struct name

#endif // BT_UTIL_H
