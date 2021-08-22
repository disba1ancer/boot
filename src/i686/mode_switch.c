#include "mode_switch.h"

I686_Descriptor I686_gdt[8] = {
    {0},
    I686_MakeSegDescriptor(0x10000, 0xFFFF, 0, I686_SegType_ExecRead | I686_DescFlag_Present),
    I686_MakeSegDescriptor(0, 0xFFFFF, 0, I686_SegType_ExecRead | I686_DescFlag_OP32 | I686_DescFlag_PageGranularity | I686_DescFlag_Present),
    I686_MakeSegDescriptor(0x10000, 0xFFFF, 0, I686_SegType_ReadWrite | I686_DescFlag_Present),
    I686_MakeSegDescriptor(0, 0xFFFFF, 0, I686_SegType_ReadWrite | I686_DescFlag_OP32 | I686_DescFlag_PageGranularity | I686_DescFlag_Present),
};
