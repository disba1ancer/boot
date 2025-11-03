#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>
#include "boot/util.h"
#include "stdalign.h"

typedef enum i686_SegTypeFlag {
    i686_SegType_TSS16 = 1 << 8,
    i686_SegType_TSS32 = 9 << 8,
    i686_SegType_TSSCommon = 1 << 8,
    i686_SegType_TSSMask = 0x15 << 8,
    i686_SegType_BusyTSS = 2 << 8,

    i686_SegType_LDT = 2 << 8,

    i686_SegType_ReadWrite = 0x12 << 8,
    i686_SegType_Read = 0x10 << 8,
    i686_SegType_Expandown = 4 << 8,

    i686_SegType_ExecRead = 0x1A << 8,
    i686_SegType_Exec = 0x18 << 8,
    i686_SegType_Conforming = 4 << 8,

    i686_SegType_Accessed = 1 << 8,
} i686_SegTypeFlag;

typedef enum i686_GateType {
    i686_GateType_G32 = 8 << 8,
    i686_GateType_Call = 0x14 << 8,
    i686_GateType_Task = 0x15 << 8,
    i686_GateType_Int = 0x16 << 8,
    i686_GateType_Trap = 0x17 << 8,
    i686_GateType_Mask = 0x14 << 8,
    i686_GateType_Common = 0x4 << 8,
} i686_GateType;

typedef enum i686_DescFlag {
    i686_DescFlag_UserFlag = 1 << 20,
    i686_DescFlag_Long = 2 << 20,
    i686_DescFlag_OP32 = 4 << 20,
    i686_DescFlag_LimitIn4K = 8 << 20,

    i686_DescFlag_Present = 1 << 15,
} i686_DescFlag;

typedef struct i686_Descriptor {
    uint32_t low;
    uint32_t high;
} i686_Descriptor;

#define i686_internal_MakeSegDescriptor(base, limit, dpl, typeflags) {\
    .low = (((uint32_t)(base) & 0xFFFFU) << 16U) | ((uint32_t)(limit) & 0xFFFFU),\
    .high = (((uint32_t)(base) >> 16U) & 0xFFU) | ((uint32_t)(base) & 0xFF000000U) | ((uint32_t)(limit) & 0xF0000U) | \
    ((uint32_t)(typeflags) & 0xF09F00) | (((uint32_t)(dpl) & 3) << 13) }

#define i686_internal_MakeGateDescriptor(offset, segsel, dpl, typeflags) {\
    .low = ((uint32_t)(offset) & 0xFFFFU) | (((uint32_t)(segsel) & 0xFFFFU) << 16U),\
    .high = ((uint32_t)(offset) & 0xFFFF0000U) | \
    ((uint32_t)(typeflags) & 0x8B00) | i686_GateType_Common | (((uint32_t)(dpl) & 3) << 13) }

#ifndef __cplusplus
#define i686_MakeSegDescriptor(base, limit, dpl, typeflags) i686_internal_MakeSegDescriptor(base, limit, dpl, typeflags)
#define i686_MakeGateDescriptor(offset, segsel, dpl, typeflags) i686_internal_MakeSegDescriptor(offset, segsel, dpl, typeflags)
#else
constexpr inline i686_Descriptor i686_MakeSegDescriptor(uint32_t base, uint32_t limit, uint32_t dpl, uint32_t typeflags)
{
    return i686_internal_MakeSegDescriptor(base, limit, dpl, typeflags);
}
#undef i686_internal_MakeSegDescriptor
constexpr inline i686_Descriptor i686_MakeGateDescriptor(uint32_t offset, uint32_t segsel, uint32_t dpl, uint32_t typeflags)
{
    return i686_internal_MakeGateDescriptor(offset, segsel, dpl, typeflags);
}
#undef i686_internal_MakeGateDescriptor
#endif

typedef enum i686_PageEntryFlag {
    i686_PageEntryFlag_Present = 1 << 0,
    i686_PageEntryFlag_Write = 1 << 1,
    i686_PageEntryFlag_User = 1 << 2,
    i686_PageEntryFlag_PWT = 1 << 3, // Write through cache
    i686_PageEntryFlag_PCD = 1 << 4, // Disable cache
    i686_PageEntryFlag_Accessed = 1 << 5,
    i686_PageEntryFlag_Dirty = 1 << 6,
    i686_PageEntryFlag_PAT = 1 << 7,
    i686_PageEntryFlag_Global = 1 << 8,
} i686_PageEntryFlag;

typedef struct i686_PageEntry {
    uint32_t data;
} i686_PageEntry;

#define i686_internal_MakePageEntry(phyPage, flags) { ((phyPage) & 0xFFFFF000U) | ((flags) & 0xFFFU) }

#ifndef __cplusplus
#define i686_MakePageEntry(phyPage, flags) i686_internal_MakePageEntry(phyPage, flags)
#else
constexpr inline i686_PageEntry i686_MakeSegDescriptor(uint32_t phyPage, uint32_t flags)
{
    return i686_internal_MakePageEntry(phyPage, flags);
}
#undef i686_internal_MakePageEntry
#endif

typedef struct x86_64_PageEntry {
    alignas(8) uint64_t data;
} x86_64_PageEntry;

#define x86_64_internal_MakePageEntry(phyPage, flags) { ((phyPage) & 0xFFFFFFFFFFFFF000U) | ((flags) & 0xFFFU) }

#ifndef __cplusplus
#define x86_64_MakePageEntry(phyPage, flags) x86_64_internal_MakePageEntry(phyPage, flags)
#else
constexpr inline x86_64_PageEntry x86_64_MakeSegDescriptor(uint64_t phyPage, uint32_t flags)
{
    return x86_64_internal_MakePageEntry(phyPage, flags);
}
#undef x86_64_internal_MakePageEntry
#endif

inline uint64_t x86_64_PageEntry_GetAddr(const x86_64_PageEntry* entry)
{
    return entry->data & 0xFFFFFFFFFFFFF000U;
}

inline uint32_t x86_64_PageEntry_GetFlags(const x86_64_PageEntry* entry)
{
    return entry->data & 0xFFFU;
}

typedef struct i686_InterruptFrame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} i686_InterruptFrame;

typedef struct i686_tss {
    uint32_t prevTask;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtSelector;
    uint16_t flags;
    uint16_t ioMap;
} i686_tss;

#define i686_Interrupt __attribute__((interrupt))

BOOT_STRUCT(i686_RMPtr) {
    uint16_t ptr;
    uint16_t seg;
};

inline void *i686_LoadPointer(i686_RMPtr fptr) {
    uintptr_t ptr = fptr.seg;
    ptr = (ptr << 4) + fptr.ptr;
    return (void*)ptr;
}

inline i686_RMPtr i686_MakeRMPointer(void *ptr) {
    uintptr_t fptr = (uintptr_t)ptr;
    i686_RMPtr rslt = { .ptr = (uint16_t)(fptr & 0xF), .seg = (uint16_t)(fptr >> 4) };
    return rslt;
}

#ifdef __cplusplus
template <typename T>
T* i686_LoadPointer(i686_RMPtr fptr) {
    return static_cast<T*>(i686_LoadPointer(fptr));
}
#endif

#endif // PROCESSOR_H
