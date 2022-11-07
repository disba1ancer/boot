#include "mode_switch.h"

i686_Descriptor i686_gdt[8] = {
    {0},
    i686_MakeSegDescriptor(0x10000, 0xFFFF, 0, i686_SegType_ExecRead | i686_DescFlag_Present),
    i686_MakeSegDescriptor(0, 0xFFFFF, 0, i686_SegType_ExecRead | i686_DescFlag_OP32 | i686_DescFlag_LimitIn4K | i686_DescFlag_Present),
    i686_MakeSegDescriptor(0x10000, 0xFFFF, 0, i686_SegType_ReadWrite | i686_DescFlag_Present),
    i686_MakeSegDescriptor(0, 0xFFFFF, 0, i686_SegType_ReadWrite | i686_DescFlag_OP32 | i686_DescFlag_LimitIn4K | i686_DescFlag_Present),
    i686_MakeSegDescriptor(0, 0xFFFFF, 0, i686_SegType_ExecRead | i686_DescFlag_Long | i686_DescFlag_LimitIn4K | i686_DescFlag_Present),
};
