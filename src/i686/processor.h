#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>

typedef enum I686_SegTypeFlag {

    I686_SegType_TSS16 = 1 << 8,
    I686_SegType_TSS32 = 9 << 8,
    I686_SegType_TSSCommon = 1 << 8,
    I686_SegType_TSSMask = 0x15 << 8,
    I686_SegType_Busy = 2 << 8,

    I686_SegType_LDT = 0x12 << 8,

    I686_SegType_ReadWrite = 0x12 << 8,
    I686_SegType_Read = 0x10 << 8,
    I686_SegType_ExpandDown = 4 << 8,

    I686_SegType_ExecRead = 0x1A << 8,
    I686_SegType_Exec = 0x18 << 8,
    I686_SegType_Conforming = 4 << 8,

    I686_SegType_Accessed = 1 << 8,
} I686_SegTypeFlag;

typedef enum I686_GateType {
    I686_GateType_G32 = 8 << 8,
    I686_GateType_Call = 0x14 << 8,
    I686_GateType_Task = 0x15 << 8,
    I686_GateType_Int = 0x16 << 8,
    I686_GateType_Trap = 0x17 << 8,
    I686_GateType_Mask = 0x14 << 8,
    I686_GateType_Common = 0x4 << 8,
} I686_GateType;

typedef enum I686_DescFlag {
    I686_DescFlag_UserFlag = 1 << 20,
    I686_DescFlag_Long = 2 << 20,
    I686_DescFlag_OP32 = 4 << 20,
    I686_DescFlag_PageGranularity = 8 << 20,

    I686_DescFlag_Present = 1 << 15,
} I686_DescFlag;

typedef struct I686_Descriptor {
    uint32_t low;
    uint32_t high;
} I686_Descriptor;

#define I686_Internal_MakeSegDescriptor(base, limit, dpl, typeflags) {\
    .low = (((uint32_t)(base) & 0xFFFFU) << 16U) | ((uint32_t)(limit) & 0xFFFFU),\
    .high = (((uint32_t)(base) >> 16U) & 0xFFU) | ((uint32_t)(base) & 0xFF000000U) | ((uint32_t)(limit) & 0xF0000U) | \
    ((uint32_t)(typeflags) & 0xF09F00) | (((uint32_t)(dpl) & 3) << 13) }

#define I686_Internal_MakeGateDescriptor(offset, segsel, dpl, typeflags) {\
    .low = ((uint32_t)(offset) & 0xFFFFU) | (((uint32_t)(segsel) & 0xFFFFU) << 16U),\
    .high = ((uint32_t)(offset) & 0xFFFF0000U) | \
    ((uint32_t)(typeflags) & 0x8B00) | I686_GateType_Common | (((uint32_t)(dpl) & 3) << 13) }

#ifndef __cplusplus
#define I686_MakeSegDescriptor(base, limit, dpl, typeflags) I686_Internal_MakeSegDescriptor(base, limit, dpl, typeflags)
#define I686_MakeGateDescriptor(offset, segsel, dpl, typeflags) I686_Internal_MakeSegDescriptor(offset, segsel, dpl, typeflags)
#else
constexpr I686_Descriptor I686_MakeSegDescriptor(uint32_t base, uint32_t limit, uint32_t dpl, uint32_t typeflags)
{
    return I686_Internal_MakeSegDescriptor(base, limit, dpl, typeflags);
}
#undef I686_Internal_MakeSegDescriptor
constexpr I686_Descriptor I686_MakeGateDescriptor(uint32_t offset, uint32_t segsel, uint32_t dpl, uint32_t typeflags)
{
    return I686_Internal_MakeGateDescriptor(offset, segsel, dpl, typeflags);
}
#undef I686_Internal_MakeGateDescriptor
#endif

typedef struct I686_InterruptFrame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} I686_InterruptFrame;

typedef struct I686_TSS {
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
} I686_TSS;

#define I686_Interrupt __attribute__((interrupt))

#endif // PROCESSOR_H
