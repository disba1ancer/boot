#ifndef MODE_SWITCH_H
#define MODE_SWITCH_H

#include "processor.h"

typedef struct i686_RealModeState {
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
} i686_RealModeState;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This function is evil, do not use it
 */
void i686_RealModeCall(i686_RealModeState* regs);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MODE_SWITCH_H
