.intel_syntax noprefix

.text
.code32
.global i686_bios_kbrd_GetKey
i686_bios_kbrd_GetKey:
        lea     esp, -8[esp]
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset getkey_rm
        jmp     I686_EnterRealMode
0:      mov     eax, 4[esp]
        mov     [eax], dx
        ret     4

.section .text16, "ax"
.code16
getkey_rm:
        xor     ax, ax
        int     0x16
        mov     dx, ax
        jmp     I686_EnterProtMode
